#include "music_box.hpp"

#include "constants.hpp"

#include <magic_enum/magic_enum.hpp>

#include <algorithm>

#include "debug_utilities.hpp"

bool MusicBox::isOutputState() const{
    return state_ == SystemState::Armed_Idle
        || state_ == SystemState::Ramping_Up
        || state_ == SystemState::Running;
}

bool MusicBox::checkProtection(){
    if(!safetyMonitor_.isLatchArmed()
    || !safetyMonitor_.isInterlockClosed()
    || safetyMonitor_.hasEdgeTripped()
    ){
        transitionTo(SystemState::Tripped);
        return true;
    }

    faultCode_ = safetyMonitor_.evaluate(sensingManager_, currentDuty_, false);
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
        currentDuty_        = .0f;
        targetDuty_         = .0f;
        isArmSignalStable_  = false;
        safetyMonitor_.clearEdgeTrip();
    } break;
    case SystemState::Armed_Idle:{
        carrierManager_.attach();
        carrierManager_.setDuty(.0f);
    } break;
    case SystemState::Tripped:
    case SystemState::Fault:{
        carrierManager_.forceOff();
        currentDuty_ = .0f;
        targetDuty_  = .0f;
    } break;
    default: break;
    }

    const auto stateName{magic_enum::enum_name(state_)};
    DEBUG_PRINT("State -> %.*s", static_cast<int>(stateName.size()), stateName.data());
}

void MusicBox::updateDisarmedState(units::TimestampUs currentTime){}

void MusicBox::updateArmedIdleState(){}

void MusicBox::updateRampingUpState(float deltaSeconds){}

void MusicBox::updateRunningState(){
    carrierManager_.setDuty(currentDuty_);
}

void MusicBox::updateTrippedState(){
    transitionTo(SystemState::Disarmed);
}

void MusicBox::updateFaultState(){
    if(safetyMonitor_.isLatchArmed()) return;
    if(safetyMonitor_.evaluate(sensingManager_, currentDuty_, true) != FaultCode::None) return;

    faultCode_ = FaultCode::None;
    transitionTo(SystemState::Disarmed);
}

#ifdef DEBUG
void MusicBox::DEBUG_printTelemetry() const{}
#endif
