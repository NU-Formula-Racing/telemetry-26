#pragma once

#include <cstdint>

namespace remote::canmsgs {

struct TelemetryRtcTimeRtcHourTraits {
  using RawType = uint8_t;
  using PhysicalType = uint8_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct TelemetryRtcTimeRtcMinuteTraits {
  using RawType = uint8_t;
  using PhysicalType = uint8_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct TelemetryRtcTimeRtcSecondTraits {
  using RawType = uint8_t;
  using PhysicalType = uint8_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct TelemetryRtcDateRtcYearTraits {
  using RawType = uint8_t;
  using PhysicalType = uint8_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct TelemetryRtcDateRtcMonthTraits {
  using RawType = uint8_t;
  using PhysicalType = uint8_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct TelemetryRtcDateRtcDayTraits {
  using RawType = uint8_t;
  using PhysicalType = uint8_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct TelemetryRtcDateRtcWeekdayTraits {
  using RawType = uint8_t;
  using PhysicalType = uint8_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct TelemetryOdometerMilesDrivenTraits {
  using RawType = uint32_t;
  using PhysicalType = float;
  static constexpr float FACTOR = 0.001F;
  static constexpr float OFFSET = 0.0F;
};

struct TelemetryStatusLogFileTraits {
  using RawType = uint16_t;
  using PhysicalType = uint16_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct TelemetryStatusLoggingStatusTraits {
  using RawType = uint8_t;
  using PhysicalType = uint8_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct TelemetryStatusWirelessHardwareStatusTraits {
  using RawType = uint8_t;
  using PhysicalType = uint8_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct TelemetryStatusWirelessProtocolStateTraits {
  using RawType = uint8_t;
  using PhysicalType = uint8_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct RearInverterMotorStatusRpmTraits {
  using RawType = int16_t;
  using PhysicalType = int16_t;
  static constexpr float FACTOR = 1.0F;
  static constexpr float OFFSET = 0.0F;
};

struct RearInverterMotorStatusMotorCurrentTraits {
  using RawType = int16_t;
  using PhysicalType = int16_t;
  static constexpr float FACTOR = 0.1F;
  static constexpr float OFFSET = 0.0F;
};

struct RearInverterMotorStatusDcVoltageTraits {
  using RawType = int16_t;
  using PhysicalType = int16_t;
  static constexpr float FACTOR = 0.1F;
  static constexpr float OFFSET = 0.0F;
};

struct RearInverterMotorStatusDcCurrentTraits {
  using RawType = int16_t;
  using PhysicalType = int16_t;
  static constexpr float FACTOR = 0.1F;
  static constexpr float OFFSET = 0.0F;
};

}  // namespace remote::canmsgs