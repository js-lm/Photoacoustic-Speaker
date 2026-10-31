#include "safety_monitor.hpp"

#include "constants.hpp"

FaultCode SafetyMonitor::evaluate(
    const SensingManager    &sensingManager,
    units::DutyCycle        duty,
    bool                    shouldApplyHysteresis
) const{
    if constexpr(constants::safety::EnableThermalProtection){
        if(!sensingManager.isNtcConnected()) return FaultCode::Ntc_Disconnected;

        const units::Celsius temperatureLimit{
            shouldApplyHysteresis
                ? constants::safety::TemperatureLimit - constants::safety::TemperatureHysteresis
                : constants::safety::TemperatureLimit
        };
        if(sensingManager.heatsinkTemperature() > temperatureLimit) return FaultCode::Over_Temperature;
    }

    if(sensingManager.peakCurrent(duty) > constants::safety::SoftPeakCurrentLimit) return FaultCode::Overcurrent;

    return FaultCode::None;
}
