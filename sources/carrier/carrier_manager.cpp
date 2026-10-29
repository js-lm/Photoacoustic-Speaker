#include "carrier_manager.hpp"

#include "constants.hpp"

#include <hardware/gpio.h>
#include <hardware/pwm.h>

void CarrierManager::initialize(){
    forceOff();
}

void CarrierManager::attach(){
    pwm_config configuration{pwm_get_default_config()};
    pwm_config_set_clkdiv_int(&configuration, divider_);
    pwm_config_set_wrap(&configuration, wrap_);

    pwm_init(constants::carrier::Slice, &configuration, false);
    pwm_set_chan_level(constants::carrier::Slice, PWM_CHAN_A, 0);

    gpio_set_function(constants::pins::PwmOutput, GPIO_FUNC_PWM);
    pwm_set_enabled(constants::carrier::Slice, true);

    isAttached_ = true;
}

void CarrierManager::forceOff(){
    gpio_init(constants::pins::PwmOutput);
    gpio_set_dir(constants::pins::PwmOutput, GPIO_OUT);
    gpio_put(constants::pins::PwmOutput, 0);

    isAttached_ = false;
}

void CarrierManager::setDuty(units::DutyCycle duty){
    if(!isAttached_) return;

    pwm_set_chan_level(constants::carrier::Slice, PWM_CHAN_A, dutyToLevel(duty));
}
