#pragma once
#include <cstdint>
#include <span>

#include "sd.hpp"
#include "utils/utils.hpp"

extern "C" {
#include "fatfs.h"
}

namespace sd {

// STM32-specific SD driver implementation using FATFS and HAL libraries
// TODO: error handling, isDetected(), read()
class Stm32SdDriver : public ISdDriver {
 public:
  Stm32SdDriver() = default;
  ~Stm32SdDriver() override = default;

  virtual SdResult init() override {
    result_ = f_mount(&sdFatFs_, SDPath, 1);

    if (result_ != FR_OK) {
      while (true) {
        ERROR("Stm32SdDriver", "Mount failed, code: ", std::to_string(result_), "\r\n");
        HAL_Delay(10);
      }
    }

    return (result_ == FR_OK) ? SdResult::OK : SdResult::ERROR;
  }

  // TODO
  virtual bool isDetected() override { return true; }

  virtual SdResult openFile(const std::string& filename, uint8_t mode) override {
    result_ = f_open(&file_, filename.data(), mode);

    if (result_ != FR_OK) {
      while (true) {
        ERROR("Stm32SdDriver", "Mount failed, code: ", std::to_string(result_), "\r\n");
        HAL_Delay(10);
      }
    }

    return (result_ == FR_OK) ? SdResult::OK : SdResult::ERROR;
  }

  virtual SdResult write(std::span<const uint8_t> data) override {
    // update pointer to end of file
    result_ = f_lseek(&file_, f_size(&file_));
    if (result_ != FR_OK) {
      return SdResult::ERROR;
    }

    // write data
    result_ = f_write(&file_, data.data(), static_cast<UINT>(data.size()), &bytesWritten_);
    return (result_ == FR_OK) ? SdResult::OK : SdResult::ERROR;
  }

  // TODO
  virtual SdResult read(std::span<uint8_t> data) override {
    static_cast<void>(data);  // avoid unused parameter warning, remove when implemented
    return SdResult::ERROR;
  }

  virtual SdResult flush() override {
    result_ = f_sync(&file_);
    return (result_ == FR_OK) ? SdResult::OK : SdResult::ERROR;
  }

 private:
  // hardware-specific members
  FATFS sdFatFs_{};
  FIL file_{};
  FRESULT result_{FR_OK};
  UINT bytesWritten_{0};
};

}  // namespace sd