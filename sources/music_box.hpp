#pragma once

#include "aliases.hpp"
#include "constants.hpp"
#include "system_states.hpp"

#include "carrier/carrier_manager.hpp"
#include "sensing/sensing_manager.hpp"
#include "safety/safety_monitor.hpp"
#include "audio/audio_streamer.hpp"
#include "usb/usb_audio_device.hpp"

class MusicBox{
private:
    UsbAudioDevice  &usbAudioDevice_{UsbAudioDevice::getInstance()};
    AudioStreamer   audioStreamer_{};
    CarrierManager  carrierManager_{};
    SensingManager  sensingManager_{};
    SafetyMonitor   safetyMonitor_{};

private:
    SystemState         state_              {SystemState::Booting};
    FaultCode           faultCode_          {FaultCode::None};

private:
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
    void updateRampingUpState();
    void updateRunningState();
    void updateRampingDownState();
    void updateTrippedState();
    void updateFaultState();

private:
    void DEBUG_printTelemetry() const;
    void DEBUG_waitForConsole();
};
