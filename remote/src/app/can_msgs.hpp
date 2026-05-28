#pragma once

#include "can_signals.hpp"
#include "can_types.hpp"

namespace remote::canmsgs {

// TX msgs
#pragma pack(push, 1)
struct TelemetryRtcTime {
  static constexpr uint32_t ID = 0x520;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  can::CanSignal<TelemetryRtcTimeRtcHourTraits> rtcHour;
  can::CanSignal<TelemetryRtcTimeRtcMinuteTraits> rtcMinute;
  can::CanSignal<TelemetryRtcTimeRtcSecondTraits> rtcSecond;
  // can::CanSignal<TelemetryRtcTimeRtcSubsecondTraits> rtcSubsecond;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TelemetryRtcDate {
  static constexpr uint32_t ID = 0x521;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  can::CanSignal<TelemetryRtcDateRtcYearTraits> rtcYear;
  can::CanSignal<TelemetryRtcDateRtcMonthTraits> rtcMonth;
  can::CanSignal<TelemetryRtcDateRtcDayTraits> rtcDay;
  can::CanSignal<TelemetryRtcDateRtcWeekdayTraits> rtcWeekday;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TelemetryOdometer {
  static constexpr uint32_t ID = 0x522;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  can::CanSignal<TelemetryOdometerMilesDrivenTraits> milesDriven;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TelemetryStatus {
  static constexpr uint32_t ID = 0x523;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  can::CanSignal<TelemetryStatusLogFileTraits> logFile;
  can::CanSignal<TelemetryStatusLoggingStatusTraits> loggingStatus;
  can::CanSignal<TelemetryStatusWirelessHardwareStatusTraits> wirelessStatus;
  can::CanSignal<TelemetryStatusWirelessProtocolStateTraits> wirelessProtocolState;

  // can::CanSignal<uint16_t, uint16_t, 1.0F, 0.0F> logFile : 16;
  // can::CanSignal<uint8_t, uint8_t, 1.0F, 0.0F> loggingStatus : 3;
  // can::CanSignal<uint8_t, uint8_t, 1.0F, 0.0F> wirelessStatus : 2;
  // can::CanSignal<uint8_t, uint8_t, 1.0F, 0.0F> wirelessProtocolState : 2;
  // std::array<uint8_t, 7> reserved;
};
#pragma pack(pop)

// RX msgs
#pragma pack(push, 1)
struct RearInverterMotorStatus {
  static constexpr uint32_t ID = 0x281;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  can::CanSignal<RearInverterMotorStatusRpmTraits> rpm;
  can::CanSignal<RearInverterMotorStatusMotorCurrentTraits> motorCurrent;
  can::CanSignal<RearInverterMotorStatusDcVoltageTraits> dcVoltage;
  can::CanSignal<RearInverterMotorStatusDcCurrentTraits> dcCurrent;
};
#pragma pack(pop)

}  // namespace remote::canmsgs