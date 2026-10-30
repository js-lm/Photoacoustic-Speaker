#pragma once

#include "aliases.hpp"
#include "constants.hpp"

class SensingManager{
private:
    units::AdcCounts        currentCounts_          {0};
    units::AdcCounts        currentZeroOffsetCounts_{0};
    units::AdcCounts        ntcCounts_              {0};
    units::TimestampUs      lastSampleTime_         {0};

public:
    SensingManager() = default;
    ~SensingManager() = default;

    void initialize();
    void update();

    // MUST be called while the output is held low so the offset is a true zero current!!!
    void calibrateZeroOffset();

public:
    units::Amperes averageCurrent() const;
    units::Amperes peakCurrent(units::DutyCycle duty) const;

    bool isNtcConnected() const;
    units::Celsius heatsinkTemperature() const;

    units::AdcCounts ntcCounts() const{ return ntcCounts_;}

private:
    void sampleChannels();
    units::AdcCounts readOversampled(physical::AdcChannel channel, uint8_t sampleCount) const;
};
