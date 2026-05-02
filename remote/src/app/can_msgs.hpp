#pragma once

#include "can.hpp"

namespace remote::canmsgs {

// TX msgs
#pragma pack(push, 1)
struct TelemetryRtcTime {
  static constexpr uint32_t ID = 0x510;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  uint8_t rtcHour;
  uint8_t rtcMinute;
  uint8_t rtcSecond;
  // uint32_t rtcSubsecond;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TelemetryRtcDate {
  static constexpr uint32_t ID = 0x511;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  uint8_t rtcYear;
  uint8_t rtcMonth;
  uint8_t rtcDay;
  uint8_t rtcWeekday;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TelemetryOdometer {
  static constexpr uint32_t ID = 0x512;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  float milesDriven;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TelemetryStatus {
  static constexpr uint32_t ID = 0x513;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  uint16_t logFile;
  uint8_t loggingStatus;
  uint8_t wirelessStatus;

  // uint16_t logFile : 16;
  // uint8_t loggingStatus : 3;
  // uint8_t wirelessStatus : 2;
  // std::array<uint8_t, 7> reserved;
};
#pragma pack(pop)

// RX msgs
#pragma pack(push, 1)
struct RearInverterMotorStatus {
  static constexpr uint32_t ID = 0x281;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  int16_t rpm;
  int16_t motorCurrent;
  int16_t dcVoltage;
  int16_t dcCurrent;
};
#pragma pack(pop)

}  // namespace remote::canmsgs