#include "audio_streamer.hpp"

#include "constants.hpp"

#include <algorithm>
#include <cmath>

#include "debug_utilities.hpp"

void AudioStreamer::resetStatistics(Statistics &statistics){
    statistics = Statistics{};
    statistics.minimumLevel = UINT32_MAX;
}

void AudioStreamer::accumulateStatistics(Statistics &total, const Statistics &window){
    total.underrunCount     += window.underrunCount;
    total.ringUnderrunCount += window.ringUnderrunCount;
    total.overflowCount     += window.overflowCount;
    total.inputSampleCount  += window.inputSampleCount;
    total.peakMagnitude      = std::max(total.peakMagnitude, window.peakMagnitude);
    total.sumOfSquares      += window.sumOfSquares;
    total.levelCount        += window.levelCount;
    total.levelSum          += window.levelSum;
    total.minimumLevel       = std::min(total.minimumLevel, window.minimumLevel);
    total.maximumLevel       = std::max(total.maximumLevel, window.maximumLevel);
}

float AudioStreamer::toDecibelsFullScale(float magnitude){
    if(magnitude < 1.0f) return -99.0f;
    return 20.0f * std::log10(magnitude / 32768.0f);
}

void AudioStreamer::DEBUG_reportGlitch(const char *format, unsigned long value){
    if(hasReportedGlitch_) return;
    hasReportedGlitch_ = true;

    DEBUG_PRINT(format, value);
}

void AudioStreamer::DEBUG_reportWindow(const UsbAudioDevice &usbAudioDevice, units::TimestampUs currentTime){
    if(currentTime - lastReportTime_ < units::Ms2Us(constants::audio::StatisticsInterval)) return;

    const float elapsedSeconds{(currentTime - lastReportTime_) / constants::system::MicrosecondsPerSecond};
    lastReportTime_ = currentTime;

    const uint32_t receivedFrames{usbAudioDevice.receivedFrames()};
    const float hostRate{(receivedFrames - reportReceivedFrames_) / elapsedSeconds};
    reportReceivedFrames_ = receivedFrames;

    const char *status{
        !isPrimed_  ? "buffering"
        : isMuted_  ? "muted"
        : isSilent_ ? "silent"
        : "playing"
    };

    const float rootMeanSquare{
        window_.inputSampleCount > 0
            ? std::sqrt(static_cast<float>(window_.sumOfSquares) / window_.inputSampleCount)
            : .0f
    };

    const bool hasLevels{window_.levelCount > 0};
    const float minimumDuty{hasLevels ? static_cast<float>(window_.minimumLevel) / periodTicks_ : .0f};
    const float maximumDuty{hasLevels ? static_cast<float>(window_.maximumLevel) / periodTicks_ : .0f};
    const float averageDuty{hasLevels ? static_cast<float>(window_.levelSum) / window_.levelCount / periodTicks_ : .0f};

    DEBUG_PRINT(
        "Audio %-9s host %5.0f Hz | fifo %3u/%u | ring %3ld | peak %5.1f rms %5.1f dBFS | duty %.3f-%.3f avg %.3f | env %.2f | underrun %lu ring %lu overflow %lu",
        status,
        hostRate,
        usbAudioDevice.availableFrames(),
        usbAudioDevice.fifoDepthFrames(),
        static_cast<long>(writtenCount_ - consumedCount()),
        toDecibelsFullScale(window_.peakMagnitude),
        toDecibelsFullScale(rootMeanSquare),
        minimumDuty,
        maximumDuty,
        averageDuty,
        static_cast<float>(envelope_) / EnvelopeUnity,
        static_cast<unsigned long>(window_.underrunCount),
        static_cast<unsigned long>(window_.ringUnderrunCount),
        static_cast<unsigned long>(window_.overflowCount)
    );

    accumulateStatistics(stream_, window_);
    resetStatistics(window_);
    hasReportedGlitch_ = false;
}

void AudioStreamer::DEBUG_reportStreamClosed(units::TimestampUs currentTime){
    accumulateStatistics(stream_, window_);
    resetStatistics(window_);

    DEBUG_PRINT(
        "Stream closed after %.1f s: %.1f s of audio, peak %.1f dBFS, underruns %lu, ring underruns %lu, overflows %lu",
        (currentTime - streamOpenedTime_) / constants::system::MicrosecondsPerSecond,
        static_cast<float>(stream_.inputSampleCount) / constants::audio::SampleRate,
        toDecibelsFullScale(stream_.peakMagnitude),
        static_cast<unsigned long>(stream_.underrunCount),
        static_cast<unsigned long>(stream_.ringUnderrunCount),
        static_cast<unsigned long>(stream_.overflowCount)
    );
}
