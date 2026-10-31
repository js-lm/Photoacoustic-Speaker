#include "safety_monitor.hpp"

#include "constants.hpp"

#include <hardware/gpio.h>

void SafetyMonitor::initialize(CarrierManager &carrierManager){
    carrierManager_ = &carrierManager;
    instance_       = this;

    gpio_init(constants::pins::InterlockStatus);
    gpio_set_dir(constants::pins::InterlockStatus, GPIO_IN);
    gpio_pull_down(constants::pins::InterlockStatus);

    gpio_init(constants::pins::LatchStatus);
    gpio_set_dir(constants::pins::LatchStatus, GPIO_IN);
    gpio_pull_down(constants::pins::LatchStatus);

    // TODO: test trip, check if gate drops to 0V
    gpio_set_irq_enabled_with_callback(constants::pins::LatchStatus, GPIO_IRQ_EDGE_FALL, true, &SafetyMonitor::handleEdge);
    gpio_set_irq_enabled(constants::pins::InterlockStatus, GPIO_IRQ_EDGE_FALL, true);
}

bool SafetyMonitor::isLatchArmed() const{
    return gpio_get(constants::pins::LatchStatus);
}

bool SafetyMonitor::isInterlockClosed() const{
    return gpio_get(constants::pins::InterlockStatus);
}

void SafetyMonitor::trip(){
    if(carrierManager_ != nullptr) carrierManager_->forceOff();

    hasEdgeTripped_.store(true);
}

void SafetyMonitor::handleEdge(unsigned int gpio, uint32_t events){
    if(instance_ != nullptr) instance_->trip();
}
