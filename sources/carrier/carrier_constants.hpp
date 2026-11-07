#pragma once

#include "aliases.hpp"

namespace constants::carrier{

    constexpr physical::PwmSlice Slice                  {0};

    constexpr units::Hz          MinimumFrequency       {1'000};
    constexpr units::Hz          MaximumFrequency       {120'000};
    constexpr units::Hz          DefaultFrequency       {48'000}; // locked to the usb audio sample rate

    constexpr units::DutyCycle   DutyCap                {.85f}; // hardware limiter trips at about 90%
    constexpr units::DutyCycle   DefaultDuty            {.25f};
    constexpr units::DutyCycle   RampRatePerSecond      {.10f};

    constexpr units::PwmWrap     MaximumWrap            {65535};
    constexpr units::ClockDivider MaximumClockDivider   {255};

} // namespace constants::carrier
