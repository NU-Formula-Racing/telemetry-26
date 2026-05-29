#pragma once

#include "can_msgs.hpp"
#include "drivers/can/can.hpp"
#include "logger.hpp"
#include "utils/utils.hpp"
#include "wireless.hpp"

namespace remote {

class Remote {
 public:
  Remote(logger::Logger& logger, wireless::Wireless& wireless, can::CanBus& canBus)
      : logger_(logger), wireless_(wireless), canBus_(canBus) {}
  ~Remote() = default;

  // delete copy and move
  Remote(const Remote&) = delete;
  Remote& operator=(const Remote&) = delete;
  Remote(Remote&&) = delete;
  Remote& operator=(Remote&&) = delete;

  void init() {
    logger_.init();
    wireless_.init();
    canBus_.init();
  }

  void processCan() {
    can::CanFrame frame{};
    while (canBus_.receive(frame, 0)) {
      logger_.logCanFrame(frame);
      wireless_.updateCanFrame(frame);
      if (frame.id == remote::canmsgs::RearInverterMotorStatus::ID) {
        auto rearInverterMotorStatus = can::decode<remote::canmsgs::RearInverterMotorStatus>(frame);
        logger_.updateOdometer(rearInverterMotorStatus.rpm, frame.timestamp);
      }
    }
  }

  void sendOdometer() {
    remote::canmsgs::TelemetryOdometer telemetryOdometer{};
    telemetryOdometer.milesDriven = logger_.getMilesDriven();

    DEBUG_OUT("Remote", GREEN,
              "Sending odometer miles: ", std::to_string(telemetryOdometer.milesDriven), "\r\n");

    can::CanFrame frame = can::encode(telemetryOdometer);
    frame.timestamp = HAL_GetTick();

    canBus_.send(frame);
    wireless_.updateCanFrame(frame);
    logger_.logCanFrame(frame);
  }

  void sendStatus() {
    remote::canmsgs::TelemetryStatus telemetryStatus{};
    logger::LoggerStatus s = logger_.getStatus();
    telemetryStatus.logFile = s.logFileIndex;
    // telemetryStatus.loggingStatus = s.loggingStatus;
    telemetryStatus.loggingStatus = 0;
    telemetryStatus.wirelessStatus = 0;

    can::CanFrame frame = can::encode(telemetryStatus);
    frame.timestamp = HAL_GetTick();

    canBus_.send(frame);
    wireless_.updateCanFrame(frame);
    logger_.logCanFrame(frame);
  }

  void sendRtcTime() {
    remote::canmsgs::TelemetryRtcTime telemetryRtcTime{};

    rtc::RtcTime time = logger_.getRtcTime();
    telemetryRtcTime.rtcHour = time.hours;
    telemetryRtcTime.rtcMinute = time.minutes;
    telemetryRtcTime.rtcSecond = time.seconds;
    telemetryRtcTime.rtcSubsecond = time.subseconds;
    // telemetryRtcTime.rtcSubsecond = HAL_GetTick();

    can::CanFrame timeFrame = can::encode(telemetryRtcTime);
    timeFrame.timestamp = HAL_GetTick();

    canBus_.send(timeFrame);
    wireless_.updateCanFrame(timeFrame);
    logger_.logCanFrame(timeFrame);
  }

  void sendRtcDate() {
    remote::canmsgs::TelemetryRtcDate telemetryRtcDate{};

    rtc::RtcDate date = logger_.getRtcDate();
    telemetryRtcDate.rtcYear = date.year;
    telemetryRtcDate.rtcMonth = date.month;
    telemetryRtcDate.rtcDay = date.day;
    telemetryRtcDate.rtcWeekday = static_cast<uint8_t>(date.weekday);

    can::CanFrame dateFrame = can::encode(telemetryRtcDate);
    dateFrame.timestamp = HAL_GetTick();

    canBus_.send(dateFrame);
    wireless_.updateCanFrame(dateFrame);
    logger_.logCanFrame(dateFrame);
  }

 private:
  logger::Logger& logger_;
  wireless::Wireless& wireless_;
  can::CanBus& canBus_;
};

class ProcessCanJob : public tasks::IJob {
 public:
  ProcessCanJob(Remote& remote) : remote_(remote) {}
  ~ProcessCanJob() override = default;

  // delete copy and move
  ProcessCanJob(const ProcessCanJob&) = delete;
  ProcessCanJob& operator=(const ProcessCanJob&) = delete;
  ProcessCanJob(ProcessCanJob&&) = delete;
  ProcessCanJob& operator=(ProcessCanJob&&) = delete;

  void init() override { remote_.init(); }

  void run() override { remote_.processCan(); }

 private:
  Remote& remote_;
};

class OdometerCanTxJob : public tasks::IJob {
 public:
  OdometerCanTxJob(Remote& remote) : remote_(remote) {}
  ~OdometerCanTxJob() override = default;

  // delete copy and move
  OdometerCanTxJob(const OdometerCanTxJob&) = delete;
  OdometerCanTxJob& operator=(const OdometerCanTxJob&) = delete;
  OdometerCanTxJob(OdometerCanTxJob&&) = delete;
  OdometerCanTxJob& operator=(OdometerCanTxJob&&) = delete;

  void init() override {}

  void run() override { remote_.sendOdometer(); }

 private:
  Remote& remote_;
};

class RtcCanTxJob : public tasks::IJob {
 public:
  RtcCanTxJob(Remote& remote) : remote_(remote) {}
  ~RtcCanTxJob() override = default;

  // delete copy and move
  RtcCanTxJob(const RtcCanTxJob&) = delete;
  RtcCanTxJob& operator=(const RtcCanTxJob&) = delete;
  RtcCanTxJob(RtcCanTxJob&&) = delete;
  RtcCanTxJob& operator=(RtcCanTxJob&&) = delete;

  void init() override {}

  void run() override {
    remote_.sendRtcTime();
    remote_.sendRtcDate();
  }

 private:
  Remote& remote_;
};

class StatusCanTxJob : public tasks::IJob {
 public:
  StatusCanTxJob(Remote& remote) : remote_(remote) {}
  ~StatusCanTxJob() override = default;

  // delete copy and move
  StatusCanTxJob(const StatusCanTxJob&) = delete;
  StatusCanTxJob& operator=(const StatusCanTxJob&) = delete;
  StatusCanTxJob(StatusCanTxJob&&) = delete;
  StatusCanTxJob& operator=(StatusCanTxJob&&) = delete;

  void init() override {}

  void run() override { remote_.sendStatus(); }

 private:
  Remote& remote_;
};

}  // namespace remote