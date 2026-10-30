#pragma once

#include "aliases.hpp"

namespace constants::safety{

    constexpr bool EnableThermalProtection{false};

    constexpr units::Celsius    TemperatureLimit            {60.0f};
    constexpr units::Celsius    TemperatureHysteresis       {5.0f};
    constexpr units::AdcCounts  NtcOpenCircuitCounts        {4000};
    constexpr units::AdcCounts  NtcShortCircuitCounts       {50};

    constexpr units::Amperes    SoftPeakCurrentLimit        {3.1f}; // below the 3.4 A hardware trip
    constexpr units::Ms         ArmDebounceDuration         {50};

} // namespace constants::safety
