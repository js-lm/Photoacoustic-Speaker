#include "sensing_manager.hpp"

#include "constants.hpp"

#include <algorithm>
#include <cmath>

units::Amperes SensingManager::averageCurrent() const{
    const units::AdcCounts countsAboveZero{
        currentCounts_ > currentZeroOffsetCounts_
            ? static_cast<units::AdcCounts>(currentCounts_ - currentZeroOffsetCounts_)
            : static_cast<units::AdcCounts>(0)
    };

    // TODO: test with multimeter, verify Iavg
    const units::Volts voltage{
        static_cast<units::Volts>(countsAboveZero) * constants::adc::ReferenceVoltage / constants::adc::MaximumCount
    };

    return voltage / constants::sensing::SenseResistance;
}

units::Amperes SensingManager::peakCurrent(units::DutyCycle duty) const{
    if(duty < constants::sensing::MinimumDutyForPeakEstimate) return .0f;

    return averageCurrent() / duty;
}

bool SensingManager::isNtcConnected() const{
    return ntcCounts_ < constants::safety::NtcOpenCircuitCounts
        && ntcCounts_ > constants::safety::NtcShortCircuitCounts;
}

units::Celsius SensingManager::heatsinkTemperature() const{
    const units::AdcCounts clampedCounts{std::clamp<units::AdcCounts>(ntcCounts_, 1, constants::adc::MaximumCount - 1)};
    
    const units::Ohms resistance{
        constants::sensing::NtcSeriesResistance * clampedCounts / (constants::adc::MaximumCount - clampedCounts)
    };

    const units::Kelvin temperature{
        1.0f / (
            1.0f / constants::sensing::NtcNominalTemperature
            + std::log(resistance / constants::sensing::NtcNominalResistance) / constants::sensing::NtcBeta
        )
    };

    return temperature - constants::sensing::KelvinOffset;
}
