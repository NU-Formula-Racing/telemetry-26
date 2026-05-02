#pragma once
#include <cstdint>
#include <span>
#include <string>

#include "Middlewares/Third_Party/FatFs/src/ff.h"
#include "Middlewares/Third_Party/FatFs/src/integer.h"
#include "etl/vector.h"
#include "sd.hpp"
#include "utils/utils.hpp"

extern "C" {
#include "fatfs.h"
}

namespace sd {

// STM32-specific SD driver implementation using FATFS and HAL libraries
class Stm32SdDriver : public ISdDriver {
 public:
  Stm32SdDriver() = default;
  ~Stm32SdDriver() override = default;

  SdResult init() override {
    result_ = f_mount(&sdFatFs_, (TCHAR const*)"0:", 1);

    if (result_ != FR_OK) {
      while (true) {
        ERROR("Stm32SdDriver", "Mount failed, code: ", std::to_string(result_), "\r\n");
        HAL_Delay(10);
      }
    }

    return (result_ == FR_OK) ? SdResult::OK : SdResult::ERROR;
  }

  // TODO
  bool isDetected() override { return true; }

  SdResult mkdir(const std::string& dirname) override {
    result_ = f_mkdir(dirname.data());

    if (result_ == FR_OK || result_ == FR_EXIST) {
      return SdResult::OK;
    }
    return SdResult::ERROR;
  }

  bool fileExists(const std::string& filename) override {
    FILINFO fno;
    result_ = f_stat(filename.data(), &fno);
    return (result_ == FR_OK);
  }

  SdResult openFile(const std::string& filename, uint8_t mode) override {
    result_ = f_open(&file_, filename.data(), mode);

    if (result_ != FR_OK) {
      while (true) {
        ERROR("Stm32SdDriver", "Mount failed, code: ", std::to_string(result_), "\r\n");
        HAL_Delay(10);
      }
    }

    return (result_ == FR_OK) ? SdResult::OK : SdResult::ERROR;
  }

  SdResult closeFile() override {
    result_ = f_close(&file_);
    return (result_ == FR_OK) ? SdResult::OK : SdResult::ERROR;
  }

  SdResult write(std::span<const uint8_t> data) override {
    result_ = f_write(&file_, data.data(), static_cast<UINT>(data.size()), &bytesWritten_);
    return (result_ == FR_OK) ? SdResult::OK : SdResult::ERROR;
  }

  SdResult read(std::span<uint8_t> data) override {
    UINT bytesRead{};
    result_ = f_read(&file_, data.data(), static_cast<UINT>(data.size()), &bytesRead);
    return (result_ == FR_OK && bytesRead == data.size()) ? SdResult::OK : SdResult::ERROR;
  }

  SdResult flush() override {
    result_ = f_sync(&file_);
    return (result_ == FR_OK) ? SdResult::OK : SdResult::ERROR;
  }

  uint32_t getFileSize(const std::string& filename) override {
    FILINFO fno;
    result_ = f_stat(filename.data(), &fno);
    if (result_ != FR_OK) {
      return 0;
    }
    return fno.fsize;
  }

  uint32_t getOpenFileSize() override { return f_size(&file_); }

  virtual SdResult seek(const uint32_t position) override {
    result_ = f_lseek(&file_, position);
    return (result_ == FR_OK) ? SdResult::OK : SdResult::ERROR;
  }

  etl::vector<std::string, MAX_NUM_DIRS> listDirectories(const std::string& path) override {
    etl::vector<std::string, MAX_NUM_DIRS> directories{};
    DIR dir;
    FILINFO fno;

    std::string targetPath = path.empty() ? "/" : path;

    result_ = f_opendir(&dir, targetPath.c_str());
    if (result_ == FR_OK) {
      while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != '\0') {
        if ((fno.fattrib & AM_DIR) != 0) {
          std::string_view name(fno.fname);
          if (name != "." && name != "..") {
            directories.push_back(std::string(name));
          }
        }
      }
      f_closedir(&dir);
    }
    return directories;
  }

  void registerTimeProvider(TimeProviderCb cb) override { timeProvider_ = cb; }

  // static helper that FATFS global C function will call
  static uint32_t getFatTime() {
    if (timeProvider_ != nullptr) {
      return timeProvider_();
    }
    // use default time of 1/1/1980 00:00:00 if no provider registered
    return 0;
  }

 private:
  // hardware-specific members
  FATFS sdFatFs_{};
  FIL file_{};
  FRESULT result_{FR_OK};
  UINT bytesWritten_{0};

  inline static TimeProviderCb timeProvider_ = nullptr;
};

}  // namespace sd

// global C function called internally by FATFS every time a file is modified
extern "C" DWORD get_fattime(void) { return sd::Stm32SdDriver::getFatTime(); }