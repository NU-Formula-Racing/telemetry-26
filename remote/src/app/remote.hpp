#pragma once

#include "drivers/can/can.hpp"
#include "logger.hpp"
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
    // float milesDriven = logger_.read(); // read miles driven from sd or nvm
    // odometer_.init(milesDriven);
  }

  void processCan() {
    can::CanFrame frame{};
    while (canBus_.receive(frame, 0)) {
      logger_.logCanFrame(frame);
      wireless_.updateCanFrame(frame);
    }
  }

  // updateCanTx() {
  // populate TelemetryRtcTime CAN msg
  // remote::can::TelemetryRtcTime telemetryRtcTime{};
  // telemetryRtcTime.hour = rtc_.getHour();
  // telemetryRtcTime.minute = rtc_.getMinute();
  // telemetryRtcTime.second = rtc_.getSecond();
  // telemetryRtcTime.subsecond = rtc_.getSubsecond();
  //
  // populate TelemetryRtcDate CAN msg
  // remote::can::TelemetryRtcDate telemetryRtcDate{};
  // telemetryRtcDate.year = rtc_.getYear();
  // telemetryRtcDate.month = rtc_.getMonth();
  // telemetryRtcDate.day = rtc_.getDay();
  // telemetryRtcDate.weekday = rtc_.getWeekday();
  //
  // populate TelemetryOdometer CAN msg
  // remote::can::TelemetryOdometer telemetryOdometer{};
  // telemetryOdometer.milesDriven = logger_.getMilesDriven();
  //
  // populate Telemetry_Status CAN msg
  // remote::can::TelemetryStatus telemetryStatus{};
  // telemetryStatus.logFile = logger_.getLogFileName();
  // telemetryStatus.loggingStatus = logger_.getStatus();
  // telemetryStatus.wirelessStatus = wireless_.getStatus();
  //
  // make a vector and make all these canmsgs part of a variant and then i can loop thru and send
  // all for (const auto& msg : canMessages_) {
  //   CanFrame encodedMsg = msg.encode();
  //   canBus_.send(encodedMsg);}
  //}

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

}  // namespace remote