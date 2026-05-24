#pragma once

#include <cstdint>

namespace servo {

class IServoDriver {
 public:
  virtual ~IServoDriver() = default;

  virtual void setAngle(uint16_t angle) = 0;
};

}  // namespace servo