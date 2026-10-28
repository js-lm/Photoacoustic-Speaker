#include "carrier_manager.hpp"

#include "constants.hpp"

void CarrierManager::initialize(){
    forceOff();
}

void CarrierManager::attach(){
    
    isAttached_ = true;
}

void CarrierManager::forceOff(){
    isAttached_ = false;
}

void CarrierManager::setDuty(units::DutyCycle duty){

}

