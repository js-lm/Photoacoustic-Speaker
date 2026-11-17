#pragma once

#include "aliases.hpp"

#include "carrier/carrier_constants.hpp"

#include <cstdint>

namespace constants::audio{

    // usb stream format. the carrier is locked to the sample rate, one pwm period per sample
    constexpr units::Hz         SampleRate              {48'000};
    constexpr uint8_t           ChannelCount            {2};
    constexpr uint8_t           BytesPerSample          {2};
    constexpr uint8_t           BitsPerSample           {16};
    constexpr uint8_t           BytesPerFrame           {ChannelCount * BytesPerSample};

    // the audio range, NOT a safety limit (that's constants::carrier::DutyCap)
    // full scale audio swings the duty between 0 and this value, silence sits at half of it
    constexpr units::DutyCycle  MaximumAudioDuty        {.40f};

    // +6 dB/octave pre-emphasis to offset the thermal roll-off of the absorber
    constexpr bool              ShouldApplyPreEmphasis  {false};
    constexpr float             PreEmphasisCoefficient  {.90f};

    // host volume control, in 1/256 dB as defined by uac2
    constexpr int16_t           MinimumVolume           {-60 * 256};
    constexpr int16_t           MaximumVolume           {0};
    constexpr int16_t           VolumeResolution        {256};

    // to prevent a loud click
    constexpr units::Ms         FadeDuration            {250};

    // darkens the led when the host keeps a stream open but plays nothing
    constexpr int32_t           SilenceThreshold        {33}; // about -60 dBFS
    constexpr units::Ms         SilenceTimeout          {2'000};

    // dma ring of pwm compare words, must be a power of two so the dma can wrap it in hardware
    constexpr uint16_t          RingLength              {1024}; // about 21 ms
    constexpr uint16_t          RingTargetFill          {512};  // about 10.7 ms of latency
    constexpr uint16_t          RingLowWatermark        {128};  // below this with an empty usb fifo is an underrun
    constexpr uint16_t          ReadChunkFrames         {96};

    // first order average of the played duty, 2^9 samples = 10.7 ms to match R24 C9 (10 ms) in front of the adc
    constexpr uint8_t           PlayedDutyAverageShift  {9};

    constexpr units::Ms         ControlReportDelay      {300}; // volume is reported once the slider settles

    static_assert((RingLength & (RingLength - 1)) == 0);
    static_assert(RingTargetFill < RingLength && RingLowWatermark < RingTargetFill);
    static_assert(MaximumAudioDuty <= constants::carrier::DutyCap);
    static_assert(SampleRate == constants::carrier::DefaultFrequency);

} // namespace constants::audio
