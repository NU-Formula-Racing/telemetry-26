#pragma once
#include <semphr.h>

#include <cstdint>
#include <span>
#include <string>

#include "etl/vector.h"

namespace sd {

enum class SdResult : uint8_t { OK, ERROR, BUSY, TIMEOUT };

enum class SdFileMode : uint8_t {
  READ = 0x01,
  WRITE = 0x02,
  OPEN_EXISTING = 0x00,
  CREATE_ALWAYS = 0x08,
  CREATE_NEW = 0x04,
  OPEN_ALWAYS = 0x10,
  OPEN_APPEND = 0x30
};

// operator overloading to allow "FileMode::WRITE | FileMode::OPEN_ALWAYS"
inline uint8_t operator|(SdFileMode a, SdFileMode b) {
  return static_cast<uint8_t>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

// function ptr type for providing file date/time metadata
using TimeProviderCb = uint32_t (*)();

class ISdDriver {
 public:
  virtual ~ISdDriver() = default;

  // initialize hardware and mount filesystem
  virtual SdResult init() = 0;

  // check if card is present
  virtual bool isDetected() = 0;

  // make a new directory
  virtual SdResult mkdir(const std::string& dirname) = 0;

  // check if a file exists on the card
  virtual bool fileExists(const std::string& filename) = 0;

  // open a file
  // filename must follow 8.3 format unless LFN is enabled
  virtual SdResult openFile(const std::string& filename, uint8_t mode) = 0;
  // read/write blocks of data
  virtual SdResult write(std::span<const uint8_t> data) = 0;
  virtual SdResult read(std::span<uint8_t> data) = 0;

  // manually flush internal buffers to ensure data is written to the card
  virtual SdResult flush() = 0;

  // register a callback to provide file date/time metadata for FATFS when creating
  // files/directories
  virtual void registerTimeProvider(TimeProviderCb cb) = 0;

  // TODO: error handling
};

// SD manager class
// TODO: read, statuses
class SdCard {
 public:
  SdCard(ISdDriver& driver) : driver_(driver) {};
  ~SdCard() = default;

  // delete copy and move
  SdCard(const SdCard&) = delete;
  SdCard& operator=(const SdCard&) = delete;
  SdCard(SdCard&&) = delete;
  SdCard& operator=(SdCard&&) = delete;

  // initialize the SD card
  SdResult init() { return driver_.init(); }

  // make a new directory
  SdResult mkdir(const std::string& dirname) { return driver_.mkdir(dirname); }

  // open a file
  // filename must follow 8.3 format unless LFN is enabled
  SdResult openFile(const std::string& filename, uint8_t mode) {
    return driver_.openFile(filename, mode);
  }

  // open a rolling log file with incremented name in format (ie. log_0000.nfr, log_0001.nfr, etc.)
  SdResult openRollingLogFile(const std::string& dateDir) {
    // try to create the directory
    if (driver_.mkdir(dateDir) == SdResult::ERROR) {
      return SdResult::ERROR;
    }

    // find the next available log file name
    uint32_t index = 0;
    std::string path;

    // find the next available filename
    // limiting index to 10000, max log is log_9999.nfr
    while (index < 10000) {
      path = dateDir + "/" + formatFilename("log_", index, ".nfr");
      if (!driver_.fileExists(path)) {
        break;
      }
      index++;
    }

    // open the file
    uint8_t mode = SdFileMode::WRITE | SdFileMode::CREATE_ALWAYS;
    return driver_.openFile(path, mode);
  }

  // write some raw data to the internal SD buffer
  // data is only written to the card with handleFlush() and driver_.flush()
  // TODO: return a status
  void write(std::span<const uint8_t> data) {
    while (!data.empty()) {
      // calculate how much space is left in active buffer and how much of the input data we can
      // copy in
      const size_t spaceRemaining = INTERNAL_BUFFER_SIZE - activeBuffer_->size();
      const size_t toCopy = std::min(spaceRemaining, data.size());

      // copy the data into the active buffer
      auto chunk = data.subspan(0, toCopy);
      activeBuffer_->insert(activeBuffer_->end(), chunk.begin(), chunk.end());

      data = data.subspan(toCopy);

      if (activeBuffer_->size() == INTERNAL_BUFFER_SIZE) {
        if (flushBuffer_ == nullptr) {
          // both buffers are full .....
          return;
        }

        // flip buffers
        flushBuffer_ = activeBuffer_;
        activeBuffer_ =
            (activeBuffer_ == &internalBufferA_) ? &internalBufferB_ : &internalBufferA_;

        xSemaphoreGive(flushSem_);  // signal flush task to write data to card
      }
    }
  }

  // TODO: read

  void handleFlush() {
    // wait for signal that buffer is full
    if (xSemaphoreTake(flushSem_, portMAX_DELAY) == pdTRUE) {
      if (flushBuffer_ != nullptr) {
        driver_.write(std::span(flushBuffer_->data(), flushBuffer_->size()));
        driver_.flush();

        flushBuffer_->clear();
        flushBuffer_ = nullptr;  // buffer is now free for use again
      }
    }
  }

  void registerTimeProvider(TimeProviderCb cb) { driver_.registerTimeProvider(cb); }

  // helper to convert date/time to FATFS format for file metadata
  static uint32_t packFatTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
                              uint8_t minute, uint8_t second) {
    // FATFS start year is 1980, need to convert from full year to FATFS format (0-119 representing
    // 1980-2099).
    // supports providing years as 2 digits (ie. 26) or 4 digits (ie. 2026)
    uint32_t fatYear = (year >= 1980) ? (year - 1980) : (year + 2000 - 1980);

    return (fatYear << 25) | (static_cast<uint32_t>(month) << 21) |
           (static_cast<uint32_t>(day) << 16) | (static_cast<uint32_t>(hour) << 11) |
           (static_cast<uint32_t>(minute) << 5) | (static_cast<uint32_t>(second) / 2);
  }

 private:
  // helper to format filenames with incrementing index, eg. log_0001.nfr
  std::string formatFilename(const std::string& prefix, uint32_t index, const std::string& suffix) {
    std::string indexStr = std::to_string(index);
    // pad with leading zeros to 4 digits
    indexStr = std::string(4 - indexStr.length(), '0') + indexStr;

    return prefix + indexStr + suffix;
  }

  ISdDriver& driver_;
  std::string filename_{};

  // 4KB internal buffers for write operations
  // aligns with standard SD allocation unit size (512 bytes per sector)
  static constexpr size_t INTERNAL_BUFFER_SIZE = 4096;
  /* alignas(32) */ etl::vector<uint8_t, INTERNAL_BUFFER_SIZE> internalBufferA_;
  /* alignas(32) */ etl::vector<uint8_t, INTERNAL_BUFFER_SIZE> internalBufferB_;

  etl::vector<uint8_t, INTERNAL_BUFFER_SIZE>* activeBuffer_ = &internalBufferA_;
  etl::vector<uint8_t, INTERNAL_BUFFER_SIZE>* flushBuffer_ = nullptr;

  SemaphoreHandle_t flushSem_ = xSemaphoreCreateBinary();
};

}  // namespace sd
