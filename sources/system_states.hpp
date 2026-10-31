#pragma once

#include <cstdint>

enum class SystemState : uint8_t{
    Booting,
    Disarmed,
    Armed_Idle,
    Running,
    Tripped,
    Fault
};

enum class FaultCode : uint8_t{
    None,
    Over_Temperature,
    Ntc_Disconnected,
    Overcurrent,
    Watchdog_Reset
};
