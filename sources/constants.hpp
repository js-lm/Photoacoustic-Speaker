#pragma once

#include "aliases.hpp"

#include "carrier/carrier_constants.hpp"
#include "sensing/sensing_constants.hpp"
#include "safety/safety_constants.hpp"

namespace constants{

    namespace pins{
        constexpr physical::Pin PwmOutput       {0};  // GP0 -> R9 -> PWM_IN
        constexpr physical::Pin InterlockStatus {14}; // GP14 <- R26 <- IL_OK
        constexpr physical::Pin LatchStatus     {15}; // GP15 <- R27 <- Q
        constexpr physical::Pin AverageCurrent  {26}; // GP26 / ADC0 <- R24 + C9 <- SENSE
        constexpr physical::Pin HeatsinkNtc     {27}; // GP27 / ADC1 <- R25 + RT1
    } // namespace pins

} // namespace constants
