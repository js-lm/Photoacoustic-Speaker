#include "sensing_manager.hpp"

#include "constants.hpp"

#include <hardware/adc.h>
#include <pico/time.h>

void SensingManager::initialize(){
    adc_init();
    adc_gpio_init(constants::pins::AverageCurrent);
    adc_gpio_init(constants::pins::HeatsinkNtc);

    calibrateZeroOffset();
    sampleChannels();
    lastSampleTime_ = time_us_64();
}

void SensingManager::update(){
    const units::TimestampUs currentTime{time_us_64()};
    if(currentTime - lastSampleTime_ < constants::sensing::SampleInterval) return;

    sampleChannels();
    lastSampleTime_ = currentTime;
}

void SensingManager::calibrateZeroOffset(){
    currentZeroOffsetCounts_ = readOversampled(
        constants::adc::AverageCurrentChannel,
        constants::sensing::ZeroOffsetSampleCount
    );
}

void SensingManager::sampleChannels(){
    currentCounts_ = readOversampled(
        constants::adc::AverageCurrentChannel, 
        constants::sensing::OversampleCount
    );
    ntcCounts_ = readOversampled(
        constants::adc::HeatsinkNtcChannel, 
        constants::sensing::OversampleCount
    );
}

units::AdcCounts SensingManager::readOversampled(physical::AdcChannel channel, uint8_t sampleCount) const{
    adc_select_input(channel);

    uint32_t sum{0};
    for(uint8_t sampleIndex{0}; sampleIndex < sampleCount; sampleIndex++){
        sum += adc_read();
    }

    return sum / sampleCount;
}
