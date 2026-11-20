#include "music_box.hpp"

#include "constants.hpp"

#include "panic_report.hpp"

#include <pico/stdlib.h>
#include <hardware/watchdog.h>

#include <magic_enum/magic_enum.hpp>

#include "debug_utilities.hpp"

MusicBox &MusicBox::getInstance(){
    static MusicBox instance{};
    return instance;
}

int MusicBox::run(){
    initialize();

    while(true){
        update();
    }

    return 0;
}

void MusicBox::initialize(){
    PanicReport::initialize();
    carrierManager_.initialize();
    usbAudioDevice_.initialize();

    stdio_init_all();

#ifdef DEBUG
    DEBUG_waitForConsole();
#endif
    DEBUG_PRINT("Fast-Switching LED Driver initializing...");

    carrierManager_.configure(constants::carrier::DefaultFrequency);
    audioStreamer_.initialize();
    sensingManager_.initialize();
    safetyMonitor_.initialize(carrierManager_);

    const bool hasWatchdogReset{watchdog_caused_reboot()};
    watchdog_enable(constants::system::WatchdogTimeout, true);

    if(hasWatchdogReset){
        faultCode_ = FaultCode::Watchdog_Reset;
        transitionTo(SystemState::Fault);
    }else{
        transitionTo(SystemState::Disarmed);
    }
}

void MusicBox::update(){
    watchdog_update();
    usbAudioDevice_.update();

    const units::TimestampUs currentTime{time_us_64()};

    sensingManager_.update();

    if(audioStreamer_.isRunning()) audioStreamer_.pump(usbAudioDevice_);
    else audioStreamer_.discard(usbAudioDevice_);

    if(isOutputState() && checkProtection()) return;

    switch(state_){
    case SystemState::Disarmed:     updateDisarmedState(currentTime);   break;
    case SystemState::Armed_Idle:   updateArmedIdleState();             break;
    case SystemState::Ramping_Up:   updateRampingUpState();             break;
    case SystemState::Running:      updateRunningState();               break;
    case SystemState::Ramping_Down: updateRampingDownState();           break;
    case SystemState::Tripped:      updateTrippedState();               break;
    case SystemState::Fault:        updateFaultState();                 break;
    default: break;
    }

#ifdef DEBUG
    DEBUG_printTelemetry();
#endif
}

void MusicBox::DEBUG_waitForConsole(){
    const units::TimestampUs endTime{time_us_64() + units::Ms2Us(constants::system::DEBUG_StartupDelay)};
    bool hasReportedPanic{PanicReport::previousPanic() == nullptr};
    while(time_us_64() < endTime){
        usbAudioDevice_.update();

        if(!hasReportedPanic && tud_cdc_connected()){
            DEBUG_PRINT("PREVIOUS BOOT PANICKED: %s", PanicReport::previousPanic());
            hasReportedPanic = true;
        }
    }
}
