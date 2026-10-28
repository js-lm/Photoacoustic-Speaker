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