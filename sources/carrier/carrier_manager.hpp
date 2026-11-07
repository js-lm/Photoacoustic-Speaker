#pragma once

#include "aliases.hpp"
#include "constants.hpp"

class CarrierManager{
private:
    units::ClockDivider divider_        {1};
    units::PwmWrap      wrap_           {constants::carrier::MaximumWrap};
    units::PwmLevel     capLevel_       {0};
    units::Hz           frequency_      {0};
    bool                isAttached_     {false};

public:
    CarrierManager() = default;
    ~CarrierManager() = default;

    void initialize();

public:
    bool configure(units::Hz requestedFrequency);
    void attach();
    // can be called from an interrupt, pulls the output low whatever the pwm is doing
    void forceOff();

    void setDuty(units::DutyCycle duty);

public:
    units::PwmLevel dutyToLevel(units::DutyCycle duty) const;

    units::Hz actualFrequency() const{ return frequency_;}
    units::PwmWrap wrap() const{ return wrap_;}
    units::PwmLevel capLevel() const{ return capLevel_;}
    bool isAttached() const{ return isAttached_;}
};
