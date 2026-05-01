#pragma once

#include <algorithm>
#include <cstdint>
#include <numbers>

namespace odometer {

struct OdometerConfig {
  float gearRatio;
  uint8_t wheelDiameter;  // in inches
};

class Integrator {
 public:
  Integrator() = default;
  ~Integrator() = default;

  float trapezoidIntegrate(const float value, const uint32_t currentTimeMs) {
    float dt = static_cast<float>(currentTimeMs - prevTimeMs_) / 1000.0F;

    // trapezoidal integration
    accumulatedSum_ += 0.5F * (value + prevValue_) * dt;

    prevValue_ = value;
    prevTimeMs_ = currentTimeMs;
    return accumulatedSum_;
  }

  void setSum(float sum) { accumulatedSum_ = sum; }

 private:
  float accumulatedSum_;
  float prevValue_ = 0.0F;
  uint32_t prevTimeMs_ = 0;
  // TODO: more integration methods (rk4, trapezoidal, etc.)
};

static constexpr float INCHES_PER_FOOT = 12.0F;
static constexpr float FEET_PER_MILE = 5280.0F;
static constexpr float SECONDS_PER_MINUTE = 60.0F;

class Odometer {
 public:
  Odometer() = default;
  ~Odometer() = default;

  // delete copy and move
  Odometer(const Odometer&) = delete;
  Odometer& operator=(const Odometer&) = delete;
  Odometer(Odometer&&) = delete;
  Odometer& operator=(Odometer&&) = delete;

  void init(const float initialMiles) {
    float initialInches = initialMiles * FEET_PER_MILE * INCHES_PER_FOOT;
    integrator_.setSum(initialInches);
    milesDriven_ = initialMiles;
  }

  void setMilesDriven(float miles) { milesDriven_ = miles; }

  float getMilesDriven() const { return milesDriven_; }

  void updateMilesDriven(const int16_t motorRpm, const uint32_t currentTimeMs) {
    float wheelRpm = static_cast<float>(std::max<int16_t>(motorRpm, 0)) / config_.gearRatio;
    // linearSpeed: in/s
    float linearSpeed = static_cast<float>(config_.wheelDiameter) * std::numbers::pi_v<float> *
                        (wheelRpm / SECONDS_PER_MINUTE);
    // distance: in
    float distance = integrator_.trapezoidIntegrate(linearSpeed, currentTimeMs);
    // milesDriven: mi
    milesDriven_ = distance / (INCHES_PER_FOOT * FEET_PER_MILE);
  }

 private:
  OdometerConfig config_{.gearRatio = 3.4F, .wheelDiameter = 16};
  Integrator integrator_;
  // if we drive >8351.3066 mi, single precision float wont have enough precision to accurately
  // accumulate smaller changes in distance
  // change to fixed point (uint32_t + DBC factor) if this becomes an issue
  float milesDriven_;
};

}  // namespace odometer