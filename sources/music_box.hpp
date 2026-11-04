#pragma once

#include "aliases.hpp"
#include "constants.hpp"
#include "system_states.hpp"

#include "carrier/carrier_manager.hpp"
#include "sensing/sensing_manager.hpp"
#include "safety/safety_monitor.hpp"

class MusicBox{
private:
    CarrierManager  carrierManager_{};
    SensingManager  sensingManager_{};
    SafetyMonitor   safetyMonitor_{};

private:
    SystemState         state_              {SystemState::Booting};
    FaultCode           faultCode_          {FaultCode::None};

    units::DutyCycle    currentDuty_        {.0f};
    units::DutyCycle    targetDuty_         {.0f};

private:
    units::TimestampUs  lastUpdateTime_     {0};
    units::TimestampUs  armSignalSince_     {0};
    bool                isArmSignalStable_  {false};

public:
    static MusicBox &getInstance();

    int run();

private:
    MusicBox() = default;
    ~MusicBox() = default;

private:
    void initialize();
    void update();

private:
    void transitionTo(SystemState nextState);
    bool isOutputState() const;
    bool checkProtection();

private:
    void updateDisarmedState(units::TimestampUs currentTime);
    void updateArmedIdleState();
    void updateRampingUpState(float deltaSeconds);
    void updateRunningState();
    void updateTrippedState();
    void updateFaultState();

private:
    void DEBUG_printTelemetry() const;
};
