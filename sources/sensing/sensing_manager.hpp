#pragma once

#include "aliases.hpp"
#include "constants.hpp"

class SensingManager{
private:
    units::TimestampUs lastSampleTime_{0};

public:
    SensingManager() = default;
    ~SensingManager() = default;

    void initialize();
    void update();

public:
    units::Amperes averageCurrent() const;
    units::Amperes peakCurrent() const;

    bool isNtcConnected() const;
    units::Celsius heatsinkTemperature() const;

private:
    void sampleChannels();
};

