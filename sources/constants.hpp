#pragma once

#include "aliases.hpp"

#include "carrier/carrier_constants.hpp"
#include "sensing/sensing_constants.hpp"
#include "safety/safety_constants.hpp"
#include "audio/audio_constants.hpp"

namespace constants{

    namespace pins{
        constexpr physical::Pin PwmOutput       {0};  // GP0 -> R9 -> PWM_IN
        constexpr physical::Pin InterlockStatus {14}; // GP14 <- R26 <- IL_OK
        constexpr physical::Pin LatchStatus     {15}; // GP15 <- R27 <- Q
        constexpr physical::Pin AverageCurrent  {26}; // GP26 / ADC0 <- R24 + C9 <- SENSE
        constexpr physical::Pin HeatsinkNtc     {27}; // GP27 / ADC1 <- R25 + RT1
    } // namespace pins

    namespace adc{
        constexpr physical::AdcChannel AverageCurrentChannel{0};
        constexpr physical::AdcChannel HeatsinkNtcChannel   {1};

        constexpr units::AdcCounts MaximumCount     {4095}; // 12 bits
        constexpr units::Volts     ReferenceVoltage {3.274f};
    } // namespace adc

    namespace system{
        constexpr units::Hz SystemClock             {125'000'000};
        constexpr units::Ms WatchdogTimeout         {100};
        constexpr units::Ms TelemetryPrintInterval  {500};
        constexpr float     MicrosecondsPerSecond   {1'000'000.0f};

        constexpr units::Ms DEBUG_StartupDelay      {2000};
    } // namespace system

} // namespace constants
