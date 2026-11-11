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

    // host volume control, in 1/256 dB as defined by uac2
    constexpr int16_t           MinimumVolume           {-60 * 256};
    constexpr int16_t           MaximumVolume           {0};
    constexpr int16_t           VolumeResolution        {256};

    constexpr uint16_t          ReadChunkFrames         {96};

    constexpr units::Ms         ControlReportDelay      {300}; // volume is reported once the slider settles

    static_assert(SampleRate == constants::carrier::DefaultFrequency);

} // namespace constants::audio
