#include "compressor.hpp"

#include "audio/audio_constants.hpp"

#include <algorithm>
#include <cmath>

namespace{
    namespace settings = constants::audio::compressor;
}

void Compressor::reset(){
    envelope_       = 0;
    gain_           = static_cast<int32_t>(std::lround((1 << GainShift) * std::pow(10.0f, targetGain(-99.0f) / 20.0f)));
    gainStep_       = 0;
    blockPosition_  = 0;

    resetWindow();
}

void Compressor::resetWindow(){
    minimumGain_    = .0f;
    maximumGain_    = .0f;
    clippedCount_   = 0;
    hasWindowGain_  = false;
}

int32_t Compressor::process(int32_t sample){
    const int32_t magnitude{(sample < 0 ? -sample : sample) << EnvelopeShift};
    const int32_t difference{magnitude - envelope_};

    envelope_ += difference > 0 ? difference >> settings::AttackShift : difference >> settings::ReleaseShift;

    if(++blockPosition_ >= settings::BlockLength){
        blockPosition_ = 0;
        updateGain();
    }
    gain_ += gainStep_;

    const int32_t output{(sample * gain_) >> GainShift};
    if(output > INT16_MAX || output < -INT16_MAX){
        clippedCount_++;

        return std::clamp<int32_t>(output, -INT16_MAX, INT16_MAX);
    }
    
    return output;
}

void Compressor::updateGain(){
    const float envelopeMagnitude{static_cast<float>(envelope_) / (1 << EnvelopeShift)};
    const float envelopeDecibels{envelopeMagnitude < 1.0f ? -99.0f : 20.0f * std::log10(envelopeMagnitude / 32768.0f)};
    const float gainDecibels{targetGain(envelopeDecibels)};

    const int32_t target{static_cast<int32_t>((1 << GainShift) * std::pow(10.0f, gainDecibels / 20.0f))};
    gainStep_ = (target - gain_) / settings::BlockLength;

    if(!hasWindowGain_){
        minimumGain_   = gainDecibels;
        maximumGain_   = gainDecibels;
        hasWindowGain_ = true;
    }else{
        minimumGain_ = std::min(minimumGain_, gainDecibels);
        maximumGain_ = std::max(maximumGain_, gainDecibels);
    }
}

float Compressor::targetGain(float envelopeDecibels){
    float gain{settings::MakeupGain};

    if(envelopeDecibels > settings::Threshold) gain -= (envelopeDecibels - settings::Threshold) * (1.0f - 1.0f / settings::Ratio);

    return std::min(gain, settings::MaximumGain);
}
