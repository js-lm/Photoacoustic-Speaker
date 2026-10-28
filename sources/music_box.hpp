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
    SystemState state_{SystemState::Booting};

private:
    units::TimestampUs  lastUpdateTime_{0};

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
    void updateRunningState();
    void updateTrippedState();
    void updateFaultState();
};
