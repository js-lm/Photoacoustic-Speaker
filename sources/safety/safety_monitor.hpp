#pragma once

#include "aliases.hpp"
#include "constants.hpp"
#include "system_states.hpp"

#include "carrier/carrier_manager.hpp"
#include "sensing/sensing_manager.hpp"

#include <atomic>
#include <cstdint>

class SafetyMonitor{
private:
    inline static SafetyMonitor *instance_{nullptr};

private:
    CarrierManager      *carrierManager_{nullptr};
    std::atomic<bool>   hasEdgeTripped_{false};

public:
    SafetyMonitor() = default;
    ~SafetyMonitor() = default;

    void initialize(CarrierManager &carrierManager);

public:
    bool isLatchArmed() const;
    bool isInterlockClosed() const;

    bool hasEdgeTripped() const{ return hasEdgeTripped_.load();}
    void clearEdgeTrip(){ hasEdgeTripped_.store(false);}

    // returns the first software detected fault, or None. hysteresis makes recovery stricter than tripping
    FaultCode evaluate(
        const SensingManager    &sensingManager,
        units::DutyCycle        duty,
        bool                    shouldApplyHysteresis
    ) const;

private:
    void trip();
    static void handleEdge(unsigned int gpio, uint32_t events);
};
