#pragma once

#include "drivers/can/can.hpp"
#include "logger.hpp"
// #include "wireless.hpp"

namespace remote {

class Remote {
 public:
  Remote(logger::Logger& logger, /*wireless::Wireless& wireless,*/ can::CanBus& canBus)
      : logger_(logger), /*wireless_(wireless),*/ canBus_(canBus) {}
  ~Remote() = default;

  // delete copy and move
  Remote(const Remote&) = delete;
  Remote& operator=(const Remote&) = delete;
  Remote(Remote&&) = delete;
  Remote& operator=(Remote&&) = delete;

  void setup() {
    logger_.setup();
    // wireless_.setup();
    canBus_.init();
  }

  void processCan() {
    can::CanFrame frame{};
    while (canBus_.receive(frame, 0)) {
      logger::LogFrame logFrame{};
      logFrame.canFrame = frame;

      logger_.logCanFrame(logFrame);
      // wireless_.updateCanFrame(frame);
    }
  }

 private:
  logger::Logger& logger_;
  // wireless::Wireless& wireless_;
  can::CanBus& canBus_;
};

class RemoteJob : public tasks::IJob {
 public:
  RemoteJob(Remote& remote) : remote_(remote) {}
  ~RemoteJob() override = default;

  // delete copy and move
  RemoteJob(const RemoteJob&) = delete;
  RemoteJob& operator=(const RemoteJob&) = delete;
  RemoteJob(RemoteJob&&) = delete;
  RemoteJob& operator=(RemoteJob&&) = delete;

  void init() override { /* remote_.setup(); */ }

  void run() override { /* remote_.processCan(); */ }

 private:
  Remote& remote_;
};

}  // namespace remote