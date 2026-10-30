#pragma once

#include "aliases.hpp"

namespace constants::sensing{

    constexpr units::Ohms       SenseResistance         {.0975f};   // 4 x 0.39 ohm in parallel
    constexpr units::Ohms       NtcSeriesResistance     {10'000.0f}; // R25
    constexpr units::Ohms       NtcNominalResistance    {10'000.0f}; // RT1 nominal temperature
    constexpr units::Kelvin     NtcNominalTemperature   {298.15f};  // 25C
    constexpr float             NtcBeta                 {3977.0f};   // Vishay NTCLE100E3103JB0
    constexpr units::Kelvin     KelvinOffset            {273.15f};

    constexpr uint8_t           OversampleCount         {16};
    constexpr uint8_t           ZeroOffsetSampleCount   {64};
    constexpr units::Us         SampleInterval          {1'000};

    constexpr units::DutyCycle  MinimumDutyForPeakEstimate  {.05f};

} // namespace constants::sensing
