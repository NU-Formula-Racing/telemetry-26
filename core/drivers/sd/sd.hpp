#pragma once
#include <cstdint>
#include <span>
#include <string>

#include "etl/vector.h"

namespace sd {

enum class SdResult : uint8_t { OK, ERROR, BUSY, TIMEOUT };

class ISdDriver {
 public:
  virtual ~ISdDriver() = default;

  // initialize hardware and mount filesystem
  virtual SdResult init() = 0;

  // check if card is present
  virtual bool isDetected() = 0;

  // open a file
  // TODO: add some argument with different modes (read, write, append, create, etc.)
  virtual SdResult openFile(const std::string& filename) = 0;

  // read/write blocks of data
  virtual SdResult write(std::span<const uint8_t> data) = 0;
  virtual SdResult read(std::span<uint8_t> data) = 0;

  // manually flush internal buffers to ensure data is written to the card
  virtual SdResult flush() = 0;

  // TODO: error handling (error enum, blink/solid LED, prints)
};

// SD manager class
// TODO: read, statuses
class SdCard {
 public:
  SdCard(ISdDriver& driver) : driver_(driver) {};

  // initialize the SD card and open a file
  // TODO: add argument(s) to specify file mode, std::string_view ?
  SdResult init(const std::string& fil) {
    //  check (preferably statically) that file name follows 8.3 format, print to enable LFN if too
    //  long
    // since this check prob has to happen at runtime for my application (but not everyone!):
    // i can call a driver function to check if LFN is enabled, return a "name too long" error code,
    // blink SD status LED a certain way and print an error
    driver_.init();
    return driver_.openFile(fil);
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
