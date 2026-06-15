#pragma once

#include <cstdint>

#include "drivers/servo/servo.hpp"

namespace aero {

static constexpr uint8_t CLOSED_ANGLE = 180;
static constexpr uint8_t OPEN_ANGLE = 20;

// replace with correct angles
enum class ServoAngle : uint8_t { CLOSED = CLOSED_ANGLE, OPEN = OPEN_ANGLE };

class ActiveAeroController {
 public:
  ActiveAeroController(servo::IServoDriver& servo) : servo_(servo) {}
  ~ActiveAeroController() = default;

  // delete copy and move
  ActiveAeroController(const ActiveAeroController&) = delete;
  ActiveAeroController& operator=(const ActiveAeroController&) = delete;
  ActiveAeroController(ActiveAeroController&&) = delete;
  ActiveAeroController& operator=(ActiveAeroController&&) = delete;

  void init() {
    servo_.init();
    servo_.setAngle(static_cast<uint16_t>(ServoAngle::CLOSED));
  }

  void setServoAngle(uint8_t commandedAirfoilState) {
    if (commandedAirfoilState == 0) {
      servoAngle_ = ServoAngle::CLOSED;
    } else if (commandedAirfoilState == 1) {
      servoAngle_ = ServoAngle::OPEN;
    } else {
      // invalid state, default to closed
      servoAngle_ = ServoAngle::CLOSED;
    }
  }

  void updateServoAngle() {
    if (servoAngle_ == ServoAngle::OPEN) {
      servo_.setAngle(static_cast<uint16_t>(ServoAngle::OPEN));
    } else {
      servo_.setAngle(static_cast<uint16_t>(ServoAngle::CLOSED));
    }
  }

  ServoAngle getServoAngle() const { return servoAngle_; }

 private:
  // prob should lock servoAngle_: processCanJob writes, activeAeroCtrlJob reads
  ServoAngle servoAngle_ = ServoAngle::CLOSED;
  servo::IServoDriver& servo_;
};

}  // namespace aero