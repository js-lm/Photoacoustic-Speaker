#include "carrier_manager.hpp"

#include "constants.hpp"

#include <algorithm>
#include <cstdint>

bool CarrierManager::configure(units::Hz requestedFrequency){
    if(requestedFrequency < constants::carrier::MinimumFrequency
    || requestedFrequency > constants::carrier::MaximumFrequency
    ) return false;

    // TODO: check gate on scope
    const uint64_t ticksPerPeriodLimit{static_cast<uint64_t>(requestedFrequency) * (constants::carrier::MaximumWrap + 1ULL)};
    const uint64_t requiredDivider{(constants::system::SystemClock + ticksPerPeriodLimit - 1) / ticksPerPeriodLimit};
    if(requiredDivider > constants::carrier::MaximumClockDivider) return false;

    divider_ = static_cast<units::ClockDivider>(std::max<uint64_t>(requiredDivider, 1));

    const uint64_t counterClock{constants::system::SystemClock / divider_};
    const uint64_t ticksPerPeriod{(counterClock + requestedFrequency / 2) / requestedFrequency};
    wrap_ = static_cast<units::PwmWrap>(ticksPerPeriod - 1);

    capLevel_ = static_cast<units::PwmLevel>(constants::carrier::DutyCap * (wrap_ + 1));
    frequency_ = static_cast<units::Hz>(counterClock / ticksPerPeriod);

    return true;
}

units::PwmLevel CarrierManager::dutyToLevel(units::DutyCycle duty) const{
    const units::DutyCycle clampedDuty{std::clamp(duty, .0f, constants::carrier::DutyCap)};
    const units::PwmLevel level{static_cast<units::PwmLevel>(clampedDuty * (wrap_ + 1) + .5f)};

    return std::min(level, capLevel_);
}
