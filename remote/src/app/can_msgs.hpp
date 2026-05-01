#pragma once

#include "can.hpp"

// define all my can msg types here

namespace remote::canmsgs {

static constexpr uint32_t TELEMETRY_RTC_TIME_ID = 0x510;
static constexpr uint32_t TELEMETRY_RTC_DATE_ID = 0x511;
static constexpr uint32_t TELEMETRY_ODOMETER_ID = 0x512;
static constexpr uint32_t TELEMETRY_STATUS_ID = 0x513;

}  // namespace remote::canmsgs