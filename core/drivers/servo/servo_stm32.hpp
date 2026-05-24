#pragma once

#include <cstdint>

#include "servo.hpp"
#include "stm32f4xx_hal.h"
#include "utils.hpp"

namespace servo {

static constexpr uint16_t MIN_ANGLE = 0;
static constexpr uint16_t MAX_ANGLE = 180;
static constexpr uint16_t MIN_PULSE_WIDTH_US = 500;
static constexpr uint16_t MAX_PULSE_WIDTH_US = 2500;

class Stm32ServoDriver : public IServoDriver {
 public:
  Stm32ServoDriver() = default;
  ~Stm32ServoDriver() override = default;

  // delete copy and move
  Stm32ServoDriver(const Stm32ServoDriver&) = delete;
  Stm32ServoDriver& operator=(const Stm32ServoDriver&) = delete;
  Stm32ServoDriver(Stm32ServoDriver&&) = delete;
  Stm32ServoDriver& operator=(Stm32ServoDriver&&) = delete;

  void setAngle(uint16_t angle) override {
    uint16_t pulseWidthUs = angleToPulseWidth(angle);

    DEBUG_OUT("StmServo", GREEN, "Setting servo angle to ", std::to_string(angle), " degrees");

    // set the compare register of the timer to update the duty cycle
    //__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulseWidthUs);
  }

 private:
  uint16_t angleToPulseWidth(const uint16_t angle) {
    return (angle * (MAX_PULSE_WIDTH_US - MIN_PULSE_WIDTH_US) / (MAX_ANGLE - MIN_ANGLE)) +
           MIN_PULSE_WIDTH_US;
  }

  // TIM_HandleTypeDef htim3;  // handle for timer used for PWM generation
};

}  // namespace servo