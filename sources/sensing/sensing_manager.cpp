#include "sensing_manager.hpp"

#include "constants.hpp"

#include <hardware/adc.h>
#include <pico/time.h>

void SensingManager::initialize(){

    lastSampleTime_ = time_us_64();
}

void SensingManager::update(){

    
}

void SensingManager::sampleChannels(){

}