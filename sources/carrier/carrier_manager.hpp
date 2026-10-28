#pragma once

#include "aliases.hpp"

class CarrierManager{
private:
    bool isAttached_{false};

public:
    CarrierManager() = default;
    ~CarrierManager() = default;

    void initialize();

public:
    void attach();
    void forceOff();

    void setDuty(units::DutyCycle duty);

public:
    bool isAttached() const{ return isAttached_;}
};

