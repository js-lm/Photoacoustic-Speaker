#include "audio_streamer.hpp"

#include "constants.hpp"

#include <algorithm>

// per sample path, integer only!

void AudioStreamer::writeFrames(const int16_t *samples, uint16_t frameCount, units::TimestampUs currentTime){
    int32_t chunkPeak{0};

    for(uint16_t frameIndex{0}; frameIndex < frameCount; frameIndex++){
        const int32_t left {samples[2 * frameIndex]};
        const int32_t right{samples[2 * frameIndex + 1]};

        // gains are at most 1 << 15, so the sum stays inside int32 and the result inside int16
        const int32_t mono{(left * leftGain_ + right * rightGain_) >> 16};

        const int32_t magnitude{mono < 0 ? -mono : mono};
        chunkPeak = std::max(chunkPeak, magnitude);
        window_.sumOfSquares += static_cast<uint32_t>(mono * mono);

        writeSample(mono);
    }

    window_.inputSampleCount += frameCount;
    window_.peakMagnitude     = std::max(window_.peakMagnitude, chunkPeak);

    updateSilenceGate(chunkPeak, currentTime);
}

void AudioStreamer::writeSilence(uint32_t sampleCount){
    while(sampleCount --> 0) writeSample(0);
}

void AudioStreamer::writeSample(int32_t sample){


    // -32768..32767 -> 0..fullScaleLevel_, silence lands on half of the audio range
    const uint32_t unsignedSample{static_cast<uint32_t>(sample + 32768)};
    uint32_t level{(unsignedSample * fullScaleLevel_) >> 16};

    if(isOutputEnabled_) envelope_ = std::min(envelope_ + envelopeStep_, EnvelopeUnity);
    else                 envelope_ = std::max<int32_t>(envelope_ - envelopeStep_, 0);

    level = (level * static_cast<uint32_t>(envelope_ >> 8)) >> 16;
    level = std::min(level, capLevel_);

    ring_[writtenCount_ & RingMask] = level;
    writtenCount_++;

    window_.levelCount++;
    window_.levelSum     += level;
    window_.minimumLevel  = std::min(window_.minimumLevel, level);
    window_.maximumLevel  = std::max(window_.maximumLevel, level);
}

void AudioStreamer::foldPlayedSamples(uint32_t consumed){
    if(consumed - playedCount_ > constants::audio::RingLength) playedCount_ = consumed - constants::audio::RingLength;

    for(; playedCount_ != consumed; playedCount_++){
        const int32_t level{static_cast<int32_t>(ring_[playedCount_ & RingMask] & 0xFFFF)};
        playedLevelAverage_ += ((level << 16) - playedLevelAverage_) >> constants::audio::PlayedDutyAverageShift;
    }
}

units::DutyCycle AudioStreamer::playedDuty() const{
    return playedLevelAverage_ / 65536.0f / periodTicks_;
}
