#pragma once
#include <cstdint>
#include <span>

#include "sd.hpp"

extern "C" {
#include "fatfs.h"
}

namespace sd {

// STM32-specific SD driver implementation using FATFS and HAL libraries
class Stm32SdDriver : public ISdDriver {
 public:
  Stm32SdDriver();
  ~Stm32SdDriver() override;

  virtual SdResult init() override;
  virtual bool isDetected() override;
  virtual SdResult openFile(const std::string& filename) override;
  virtual SdResult write(std::span<const uint8_t> data) override;
  virtual SdResult read(std::span<uint8_t> data) override;
  virtual SdResult flush() override;

 private:
  // hardware-specific members
};

}  // namespace sd