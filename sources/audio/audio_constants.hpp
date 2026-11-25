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
    // fixed duty / variable current experiments found the loudest output at around this duty
    constexpr units::DutyCycle  MaximumAudioDuty        {.75f};


    // host volume control, in 1/256 dB as defined by uac2
    constexpr int16_t           MinimumVolume           {-60 * 256};
    constexpr int16_t           MaximumVolume           {0};
    constexpr int16_t           VolumeResolution        {256};

    // try to lift the music to use more of the duty range
    namespace compressor{
        constexpr bool      IsEnabled       {true};
        constexpr float     Threshold       {-30.0f};   // dBFS, the curve bends here
        constexpr float     Ratio           {4.0f};     // above the threshold, 4 dB in -> 1 dB out
        constexpr float     OutputCeiling   {-1.0f};    // dBFS a full scale input comes out at
        constexpr float     MaximumGain     {24.0f};    // dB, caps the lift of quiet passages and noise
        constexpr uint8_t   AttackShift     {4};        // 2^4 samples, about 0.3 ms
        constexpr uint8_t   ReleaseShift    {14};       // 2^14 samples, about 340 ms
        constexpr uint16_t  BlockLength     {32};       // samples per gain update, about 0.7 ms

        // gain below the threshold, so a full scale input lands on OutputCeiling
        constexpr float     MakeupGain      {OutputCeiling - Threshold * (1.0f - 1.0f / Ratio)};

        static_assert(Ratio >= 1.0f && Threshold < OutputCeiling);
    } // namespace compressor

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

    constexpr units::Ms         StatisticsInterval      {1'000};
    constexpr units::Ms         ControlReportDelay      {300}; // volume is reported once the slider settles

    static_assert((RingLength & (RingLength - 1)) == 0);
    static_assert(RingTargetFill < RingLength && RingLowWatermark < RingTargetFill);
    static_assert(MaximumAudioDuty <= constants::carrier::DutyCap);
    static_assert(SampleRate == constants::carrier::DefaultFrequency);

} // namespace constants::audio
