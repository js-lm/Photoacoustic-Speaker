#include "music_box.hpp"

#include "constants.hpp"

#include <magic_enum/magic_enum.hpp>

#include "debug_utilities.hpp"

bool MusicBox::isOutputState() const{
    return state_ == SystemState::Armed_Idle
        || state_ == SystemState::Ramping_Up
        || state_ == SystemState::Running
        || state_ == SystemState::Ramping_Down;
}

bool MusicBox::checkProtection(){
    if(!safetyMonitor_.isLatchArmed()
    || !safetyMonitor_.isInterlockClosed()
    || safetyMonitor_.hasEdgeTripped()
    ){
        transitionTo(SystemState::Tripped);
        return true;
    }

    faultCode_ = safetyMonitor_.evaluate(sensingManager_, audioStreamer_.playedDuty(), false);
    if(faultCode_ != FaultCode::None){
        transitionTo(SystemState::Fault);
        return true;
    }

    return false;
}

void MusicBox::transitionTo(SystemState nextState){
    state_ = nextState;

    switch(nextState){
    case SystemState::Disarmed:{
        carrierManager_.forceOff();
        audioStreamer_.stop();
        isArmSignalStable_ = false;
        safetyMonitor_.clearEdgeTrip();
    } break;
    case SystemState::Armed_Idle:{
        if(!audioStreamer_.isRunning()){
            carrierManager_.attach();
            audioStreamer_.start(carrierManager_);
        }
        audioStreamer_.setOutputEnabled(false);
    } break;
    case SystemState::Ramping_Up:   audioStreamer_.setOutputEnabled(true);  break;
    case SystemState::Ramping_Down: audioStreamer_.setOutputEnabled(false); break;
    case SystemState::Tripped:
    case SystemState::Fault:{
        carrierManager_.forceOff();
        audioStreamer_.stop();
    } break;
    default: break;
    }

    const auto stateName{magic_enum::enum_name(state_)};
    DEBUG_PRINT("State -> %.*s", static_cast<int>(stateName.size()), stateName.data());
}

void MusicBox::updateDisarmedState(units::TimestampUs currentTime){
    safetyMonitor_.clearEdgeTrip();

    if(!safetyMonitor_.isLatchArmed() || !safetyMonitor_.isInterlockClosed()){
        isArmSignalStable_ = false;
        return;
    }

    if(!isArmSignalStable_){
        isArmSignalStable_ = true;
        armSignalSince_    = currentTime;
        return;
    }

    if(currentTime - armSignalSince_ >= units::Ms2Us(constants::safety::ArmDebounceDuration)){
        transitionTo(SystemState::Armed_Idle);
    }
}

void MusicBox::updateArmedIdleState(){
    if(audioStreamer_.hasSignal()) transitionTo(SystemState::Ramping_Up);
}

void MusicBox::updateRampingUpState(){
    if(!audioStreamer_.hasSignal()) transitionTo(SystemState::Ramping_Down);
    else if(audioStreamer_.isFadedIn()) transitionTo(SystemState::Running);
}

void MusicBox::updateRunningState(){
    if(!audioStreamer_.hasSignal()) transitionTo(SystemState::Ramping_Down);
}

void MusicBox::updateRampingDownState(){
    if(audioStreamer_.hasSignal()) transitionTo(SystemState::Ramping_Up);
    else if(audioStreamer_.isFadedOut()) transitionTo(SystemState::Armed_Idle);
}

void MusicBox::updateTrippedState(){
    transitionTo(SystemState::Disarmed);
}

void MusicBox::updateFaultState(){
    if(safetyMonitor_.isLatchArmed()) return;
    if(safetyMonitor_.evaluate(sensingManager_, .0f, true) != FaultCode::None) return;

    faultCode_ = FaultCode::None;
    transitionTo(SystemState::Disarmed);
}

void MusicBox::DEBUG_printTelemetry() const{
    const auto stateName{magic_enum::enum_name(state_)};
    const auto faultName{magic_enum::enum_name(faultCode_)};
    const units::DutyCycle playedDuty{audioStreamer_.playedDuty()};

    DEBUG_PRINT_TIMED(
        constants::system::TelemetryPrintInterval,
        "%.*s fault=%.*s duty=%.3f latch=%d interlock=%d Iavg=%.3fA Ipk=%.2fA ntc=%u T=%.1fC",
        static_cast<int>(stateName.size()), stateName.data(),
        static_cast<int>(faultName.size()), faultName.data(),
        playedDuty,
        safetyMonitor_.isLatchArmed(),
        safetyMonitor_.isInterlockClosed(),
        sensingManager_.averageCurrent(),
        sensingManager_.peakCurrent(playedDuty),
        sensingManager_.ntcCounts(),
        sensingManager_.heatsinkTemperature()
    );
}
