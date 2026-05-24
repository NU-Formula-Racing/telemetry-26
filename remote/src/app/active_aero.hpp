#pragma once

#include <cstdint>

#include "drivers/servo/servo.hpp"

namespace aero {

static constexpr int32_t OPEN_DRS_THRESHOLD = 360000 / 4;

// replace with correct angles
enum class ServoAngle : uint8_t { CLOSED = 50, OPEN = 120 };

class ActiveAeroController {
 public:
  ActiveAeroController(servo::IServoDriver& servo) : servo_(servo) {}
  ~ActiveAeroController() = default;

  // delete copy and move
  ActiveAeroController(const ActiveAeroController&) = delete;
  ActiveAeroController& operator=(const ActiveAeroController&) = delete;
  ActiveAeroController(ActiveAeroController&&) = delete;
  ActiveAeroController& operator=(ActiveAeroController&&) = delete;

  void setCurrentRequest(int32_t currentRequest) { currentRequest_ = currentRequest; }

  void updateServoAngle() {
    if (currentRequest_ > OPEN_DRS_THRESHOLD) {
      servo_.setAngle(static_cast<uint16_t>(ServoAngle::OPEN));
    } else {
      servo_.setAngle(static_cast<uint16_t>(ServoAngle::CLOSED));
    }
  }

  ServoAngle getServoAngle() const { return servoAngle_; }

 private:
  // prob should lock currentRequest_: processCanJob writes, activeAeroCtrlJob reads
  int32_t currentRequest_ = 0;
  ServoAngle servoAngle_ = ServoAngle::CLOSED;
  servo::IServoDriver& servo_;
};

}  // namespace aero