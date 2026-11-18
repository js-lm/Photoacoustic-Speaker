#include "audio_streamer.hpp"

#include "constants.hpp"

#include <hardware/dma.h>
#include <hardware/pwm.h>
#include <pico/time.h>

#include <algorithm>
#include <cmath>

#include "debug_utilities.hpp"

void AudioStreamer::initialize(){
    dmaChannel_ = dma_claim_unused_channel(true);
}

void AudioStreamer::start(const CarrierManager &carrierManager){
    periodTicks_    = carrierManager.wrap() + 1u;
    fullScaleLevel_ = static_cast<uint32_t>(constants::audio::MaximumAudioDuty * periodTicks_ + .5f);
    capLevel_       = carrierManager.capLevel();
    envelopeStep_   = EnvelopeUnity / static_cast<int32_t>(constants::audio::FadeDuration * constants::audio::SampleRate / 1000);

    isOutputEnabled_ = false;
    isPrimed_        = false;
    primingStartTime_= time_us_64();

    startDma();
    isRunning_ = true;

    DEBUG_PRINT(
        "Audio output ready: carrier %lu Hz, %lu levels (%.1f bits), full scale duty %.2f, %.1f ms buffer",
        static_cast<unsigned long>(carrierManager.actualFrequency()),
        static_cast<unsigned long>(fullScaleLevel_),
        std::log2(static_cast<float>(fullScaleLevel_)),
        constants::audio::MaximumAudioDuty,
        constants::audio::RingTargetFill * 1000.0f / constants::audio::SampleRate
    );
}

void AudioStreamer::stop(){
    if(!isRunning_) return;

    dma_channel_abort(dmaChannel_);

    isRunning_          = false;
    isOutputEnabled_    = false;
    isPrimed_           = false;
    envelope_           = 0;
    playedLevelAverage_ = 0;
}

void AudioStreamer::startDma(){
    dma_channel_abort(dmaChannel_);

    std::fill_n(ring_, constants::audio::RingLength, 0u);
    writtenCount_       = constants::audio::RingTargetFill;
    playedCount_        = 0;
    playedLevelAverage_ = 0;
    envelope_           = 0;
    previousSample_     = 0;

    dma_channel_config configuration{dma_channel_get_default_config(dmaChannel_)};
    channel_config_set_transfer_data_size(&configuration, DMA_SIZE_32);
    channel_config_set_read_increment(&configuration, true);
    channel_config_set_write_increment(&configuration, false);
    channel_config_set_ring(&configuration, false, RingAddressBits);
    channel_config_set_dreq(&configuration, pwm_get_dreq(constants::carrier::Slice));

    dma_channel_configure(
        dmaChannel_,
        &configuration,
        &pwm_hw->slice[constants::carrier::Slice].cc,
        ring_,
        UINT32_MAX,
        true
    );
}

uint32_t AudioStreamer::consumedCount() const{
    return UINT32_MAX - dma_channel_hw_addr(dmaChannel_)->transfer_count;
}

void AudioStreamer::pump(UsbAudioDevice &usbAudioDevice){
    if(!isRunning_) return;

    const units::TimestampUs currentTime{time_us_64()};
    trackStream(usbAudioDevice, currentTime);
    isMuted_   = usbAudioDevice.isMuted();
    leftGain_  = usbAudioDevice.leftGain();
    rightGain_ = usbAudioDevice.rightGain();

    if(!dma_channel_is_busy(dmaChannel_)){
        DEBUG_PRINT("Audio: dma transfer count somehow exhausted (what are you doing), restarting the ring");
        startDma();

        return;
    }

    const uint32_t consumed{consumedCount()};
    foldPlayedSamples(consumed);

    int32_t fill{static_cast<int32_t>(writtenCount_ - consumed)};
    if(fill < 0){
        window_.ringUnderrunCount++;
        DEBUG_reportGlitch("Ring underrun: main loop stalled, %lu stale samples replayed", static_cast<unsigned long>(-fill));
        writtenCount_ = consumed;
        fill = 0;
    }

    if(isStreamOpen_ && !isPrimed_) updatePriming(usbAudioDevice, currentTime);

    if(isStreamOpen_ && usbAudioDevice.checkOverflow()){
        window_.overflowCount++;
        DEBUG_reportGlitch("Overflow: usb fifo full, host data dropped (ring fill %lu)", static_cast<unsigned long>(fill));
    }

    if(fill < constants::audio::RingTargetFill){
        uint32_t neededCount{constants::audio::RingTargetFill - static_cast<uint32_t>(fill)};

        if(isStreamOpen_ && isPrimed_){
            while(neededCount > 0){
                const uint16_t frameCount{usbAudioDevice.read(
                    readBuffer_,
                    std::min<uint32_t>(neededCount, constants::audio::ReadChunkFrames)
                )};
                if(frameCount == 0) break;

                writeFrames(readBuffer_, frameCount, currentTime);
                neededCount -= frameCount;
            }

            const uint32_t remainingFill{constants::audio::RingTargetFill - neededCount};
            if(neededCount > 0 && remainingFill < constants::audio::RingLowWatermark){
                window_.underrunCount++;
                isPrimed_         = false;
                primingStartTime_ = currentTime;
                DEBUG_reportGlitch("Underrun: usb fifo empty with %lu samples left, re-buffering", static_cast<unsigned long>(remainingFill));
                writeSilence(neededCount);
            }
        }else{
            writeSilence(neededCount);
        }
    }

#ifdef DEBUG
    if(isStreamOpen_) DEBUG_reportWindow(usbAudioDevice, currentTime);
#endif
}

void AudioStreamer::discard(UsbAudioDevice &usbAudioDevice){
    const units::TimestampUs currentTime{time_us_64()};
    trackStream(usbAudioDevice, currentTime);
    isMuted_ = usbAudioDevice.isMuted();

    if(!isStreamOpen_) return;

    const uint16_t availableFrames{usbAudioDevice.availableFrames()};
    const uint16_t regulationFrames{static_cast<uint16_t>(usbAudioDevice.fifoDepthFrames() / 2)};
    if(availableFrames > regulationFrames) usbAudioDevice.discard(availableFrames - regulationFrames);

    usbAudioDevice.checkOverflow();
}

void AudioStreamer::trackStream(const UsbAudioDevice &usbAudioDevice, units::TimestampUs currentTime){
    const bool isOpen{usbAudioDevice.isStreamOpen()};
    if(isOpen == isStreamOpen_) return;

    isStreamOpen_ = isOpen;
    isPrimed_     = false;

    if(!isOpen){
        DEBUG_reportStreamClosed(currentTime);
        return;
    }

    streamOpenedTime_       = currentTime;
    primingStartTime_       = currentTime;
    lastSignalTime_         = currentTime;
    lastReportTime_         = currentTime;
    reportReceivedFrames_   = usbAudioDevice.receivedFrames();
    isSilent_               = false;
    hasReportedGlitch_      = false;
    resetStatistics(window_);
    resetStatistics(stream_);

    DEBUG_PRINT(
        "Stream opened: %lu Hz, %u ch, %u bit%s",
        static_cast<unsigned long>(constants::audio::SampleRate),
        constants::audio::ChannelCount,
        constants::audio::BitsPerSample,
        isRunning_ ? "" : " -- driver not armed, audio is discarded until ARM"
    );
}

void AudioStreamer::updatePriming(const UsbAudioDevice &usbAudioDevice, units::TimestampUs currentTime){
    const uint16_t availableFrames{usbAudioDevice.availableFrames()};
    const uint16_t depthFrames{usbAudioDevice.fifoDepthFrames()};
    if(availableFrames < depthFrames / 2) return;

    isPrimed_ = true;

    const float waitedMilliseconds{(currentTime - primingStartTime_) / 1000.0f};
    const bool isRecovering{window_.underrunCount + stream_.underrunCount > 0};
    DEBUG_PRINT(
        "%s after %.1f ms (usb fifo %u/%u frames)",
        isRecovering ? "Recovered from underrun" : "Stream buffered, playing",
        waitedMilliseconds,
        availableFrames,
        depthFrames
    );
}

void AudioStreamer::updateSilenceGate(int32_t chunkPeak, units::TimestampUs currentTime){
    if(chunkPeak > constants::audio::SilenceThreshold){
        lastSignalTime_ = currentTime;
        if(isSilent_){
            isSilent_ = false;
            DEBUG_PRINT("Signal detected (peak %.1f dBFS), fading in", toDecibelsFullScale(chunkPeak));
        }
        return;
    }

    if(!isSilent_ && currentTime - lastSignalTime_ >= units::Ms2Us(constants::audio::SilenceTimeout)){
        isSilent_ = true;
        DEBUG_PRINT("Silent for %.1f s, fading out", constants::audio::SilenceTimeout / 1000.0f);
    }
}
