#pragma once
#include <cstdint>
#include <span>
#include <string>

#include "etl/vector.h"

namespace sd {

enum class SdResult : uint8_t { OK, ERROR, BUSY, TIMEOUT };

// TODO: should these have corresponding values
enum class SdFileMode : uint8_t {
  READ,
  WRITE,
  OPEN_EXISTING,
  CREATE_ALWAYS,
  CREATE_NEW,
  OPEN_ALWAYS,
  OPEN_APPEND
};

// operator overloading to allow "FileMode::WRITE | FileMode::OPEN_ALWAYS"
inline SdFileMode operator|(SdFileMode a, SdFileMode b) {
  return static_cast<SdFileMode>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

class ISdDriver {
 public:
  virtual ~ISdDriver() = default;

  // initialize hardware and mount filesystem
  virtual SdResult init() = 0;

  // check if card is present
  virtual bool isDetected() = 0;

  // open a file
  // filename must follow 8.3 format unless LFN is enabled
  virtual SdResult openFile(const std::string& filename, SdFileMode mode) = 0;

  // read/write blocks of data
  virtual SdResult write(std::span<const uint8_t> data) = 0;
  virtual SdResult read(std::span<uint8_t> data) = 0;

  // manually flush internal buffers to ensure data is written to the card
  virtual SdResult flush() = 0;

  // TODO: error handling (error enum, blink/solid LED, prints)
};

// SD manager class
// TODO: read, statuses, directories (ie. new directory for each drive day), automatically correctly
// increment file names
class SdCard {
 public:
  SdCard(ISdDriver& driver) : driver_(driver) {};

  // initialize the SD card and open a file
  // filename must follow 8.3 format unless LFN is enabled
  SdResult init(const std::string& filename, SdFileMode mode) {
    driver_.init();
    return driver_.openFile(filename, mode);
  }

  // write some raw data to the internal SD buffer
  // data is only written to the card with flushBuffer() and driver_.flush()
  // TODO: return a status
  void write(std::span<const uint8_t> data) {
    // check that data fits into internal buffer
    if (internalBuffer_.size() + data.size() > INTERNAL_BUFFER_SIZE) {
      flushBuffer();  // send full chunk to card
    }
    // copy data to internal buffer
    for (const auto& i : data) {
      internalBuffer_.push_back(i);
    }
  }

  // TODO: read

  // function to be called periodically to flush internal buffer to the card
  // every 200-500ms for now
  void periodicSync() {
    if (!internalBuffer_.empty()) {
      flushBuffer();
    }
    driver_.flush();
  }

 private:
  // send internal buffer to the card and clear it
  void flushBuffer() {
    driver_.write(std::span<const uint8_t>(internalBuffer_.data(), internalBuffer_.size()));
    internalBuffer_.clear();
  }

  ISdDriver& driver_;

  // 512B internal buffer for write operations
  // aligns with standard SD allocation unit size (512 bytes per sector)
  // TODO: (maybe) ping pong buffering, increase size to something like 4KB (8 sectors) later
  static constexpr size_t INTERNAL_BUFFER_SIZE = 512;
  etl::vector<uint8_t, INTERNAL_BUFFER_SIZE> internalBuffer_;
};

}  // namespace sd
