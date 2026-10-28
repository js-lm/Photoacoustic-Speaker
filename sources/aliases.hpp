#pragma once

#include <cstdint>

namespace units{

    using Ms = uint32_t;
    using Us = uint64_t;

    inline constexpr Us Ms2Us(Ms millisecond){ return static_cast<Us>(millisecond) * 1000;}
    inline constexpr Ms Us2Ms(Us microsecond){ return static_cast<Ms>(microsecond / 1000);}
    
    using TimestampMs = uint64_t;
    using TimestampUs = uint64_t;

    // Percentage in float (.0f to 1.0f)
    using PercentageF = float;
    // Percentage in integer (0 to 100)
    // - Percentage = (float)(integer / 100.0f)
    using Percentage = uint8_t;

    using Hz  = uint32_t;
    using HzF = float;

    // pwm
    using DutyCycle    = PercentageF;   // .0f to 1.0f of the carrier period
    using PwmLevel     = uint16_t;      // compare value, in counter ticks
    using PwmWrap      = uint16_t;      // counter top value, in counter ticks
    using ClockDivider = uint8_t;       // integer pwm clock divider

    // electrical and thermal sensing
    using AdcCounts = uint16_t; // 12 bit reading, raw or oversampled
    using Volts     = float;
    using Amperes   = float;
    using Ohms      = float;
    using Celsius   = float;
    using Kelvin    = float;

} // namespace units

namespace physical{

    using Pin        = uint8_t;
    using AdcChannel = uint8_t;
    using PwmSlice   = uint8_t;

} // namespace physical
