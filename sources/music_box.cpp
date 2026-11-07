#include "music_box.hpp"

#include "constants.hpp"

#include <pico/stdlib.h>
#include <hardware/watchdog.h>
#include <hardware/clocks.h>

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
    carrierManager_.initialize();

    stdio_init_all();
    DEBUG_PRINT("Fast-Switching LED Driver initializing...");
    DEBUG_SLEEP_MS(constants::system::DEBUG_StartupDelay);

    if(clock_get_hz(clk_sys) != constants::system::SystemClock){
        DEBUG_PRINT(
            "WARNING: system clock is %lu Hz, carrier math assumes %lu Hz",
            static_cast<unsigned long>(clock_get_hz(clk_sys)),
            static_cast<unsigned long>(constants::system::SystemClock)
        );
    }

    carrierManager_.configure(constants::carrier::DefaultFrequency);
    sensingManager_.initialize();
    safetyMonitor_.initialize(carrierManager_);

    lastUpdateTime_ = time_us_64();

    // TODO: test watchdog, see if reboot flags and safe fallback to disarmed or fault state
    const bool hasWatchdogReset{watchdog_caused_reboot()};
    watchdog_enable(constants::system::WatchdogTimeout, true);

    if(hasWatchdogReset){
        faultCode_ = FaultCode::Watchdog_Reset;
        transitionTo(SystemState::Fault);
    }else{
        transitionTo(SystemState::Disarmed);
    }

    DEBUG_PRINT("Carrier set to %lu Hz", static_cast<unsigned long>(carrierManager_.actualFrequency()));
}

void MusicBox::update(){
    watchdog_update();

    const units::TimestampUs currentTime{time_us_64()};
    const float deltaSeconds{(currentTime - lastUpdateTime_) / constants::system::MicrosecondsPerSecond};
    lastUpdateTime_ = currentTime;

    sensingManager_.update();

    if(isOutputState() && checkProtection()) return;

    switch(state_){
    case SystemState::Disarmed:     updateDisarmedState(currentTime);   break;
    case SystemState::Armed_Idle:   updateArmedIdleState();             break;
    case SystemState::Ramping_Up:   updateRampingUpState(deltaSeconds); break;
    case SystemState::Running:      updateRunningState();               break;
    case SystemState::Tripped:      updateTrippedState();               break;
    case SystemState::Fault:        updateFaultState();                 break;
    default: break;
    }

#ifdef DEBUG
    DEBUG_printTelemetry();
#endif
}
