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
  Stm32ServoDriver(TIM_HandleTypeDef* htim, uint32_t channel) : htim_(htim), channel_(channel) {}
  ~Stm32ServoDriver() override = default;

  // delete copy and move
  Stm32ServoDriver(const Stm32ServoDriver&) = delete;
  Stm32ServoDriver& operator=(const Stm32ServoDriver&) = delete;
  Stm32ServoDriver(Stm32ServoDriver&&) = delete;
  Stm32ServoDriver& operator=(Stm32ServoDriver&&) = delete;

  void init() {
    __HAL_TIM_MOE_ENABLE(htim_);
    HAL_TIM_PWM_Start(htim_, channel_);
    if (htim_ == nullptr || htim_->Instance == nullptr) {
      DEBUG_OUT("StmServo", RED, "htim or htim->Instance is null\r\n");
      return;
    }
  }

  void setAngle(uint16_t angle) override {
    if (htim_ == nullptr || htim_->Instance == nullptr) {
      DEBUG_OUT("StmServo", RED, "htim or htim->Instance is null\r\n");
      return;
    }

    if (angle < MIN_ANGLE) {
      angle = MIN_ANGLE;
    } else if (angle > MAX_ANGLE) {
      angle = MAX_ANGLE;
    }

    uint16_t pulseWidthUs = angleToPulseWidth(angle);

    DEBUG_OUT("StmServo", BLUE, "Setting servo angle to ", std::to_string(angle),
              " degrees on channel ", std::to_string(channel_), "\r\n");

    __HAL_TIM_SET_COMPARE(htim_, channel_, pulseWidthUs);
  }

 private:
  uint16_t angleToPulseWidth(const uint16_t angle) {
    return (angle * (MAX_PULSE_WIDTH_US - MIN_PULSE_WIDTH_US) / (MAX_ANGLE - MIN_ANGLE)) +
           MIN_PULSE_WIDTH_US;
  }

  TIM_HandleTypeDef* htim_;
  uint32_t channel_;
};

}  // namespace servo