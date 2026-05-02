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

    logger_.logCanFrame(frame);
    wireless_.updateCanFrame(frame);
    canBus_.send(frame);
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

}  // namespace remote