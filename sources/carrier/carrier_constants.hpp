#pragma once

#include "aliases.hpp"

namespace constants::carrier{

    constexpr physical::PwmSlice Slice                  {0};

    constexpr units::DutyCycle   DutyCap                {.85f}; // hardware limiter trips at about 90%
    constexpr units::DutyCycle   DefaultDuty            {.5f};
    constexpr units::DutyCycle   RampRatePerSecond      {.0f};

} // namespace constants::carrier

