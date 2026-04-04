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
  }

  void processCan() {
    can::CanFrame frame{};
    while (canBus_.receive(frame, 0)) {
      logger_.logCanFrame(frame);
      wireless_.updateCanFrame(frame);
    }
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

}  // namespace remote