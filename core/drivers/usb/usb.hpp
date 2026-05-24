#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#ifdef __arm__
#include <FreeRTOS.h>
#include <stream_buffer.h>
#else
// Mock out FreeRTOS types for native builds
using StreamBufferHandle_t = void*;
constexpr uint32_t portMAX_DELAY = 0xFFFFFFFF;
inline StreamBufferHandle_t xStreamBufferCreate(size_t, size_t) { return nullptr; }
inline size_t xStreamBufferSend(StreamBufferHandle_t, const void*, size_t, uint32_t) { return 0; }
inline size_t xStreamBufferReceive(StreamBufferHandle_t, void*, size_t, uint32_t) { return 0; }
#endif

namespace usb {

class IUsbDriver {
 public:
  virtual ~IUsbDriver() = default;

  virtual void init() = 0;

  virtual void write(std::span<const uint8_t> data) = 0;
};

class Usb {
 public:
  Usb(IUsbDriver& driver) : driver_(driver) {}
  ~Usb() = default;

  Usb(const Usb&) = delete;
  Usb& operator=(const Usb&) = delete;
  Usb(Usb&&) = delete;
  Usb& operator=(Usb&&) = delete;

  void init() {
    streamBuffer_ = xStreamBufferCreate(2048, 1);
    driver_.init();
  }

  void write(std::span<const uint8_t> data) {
    if (streamBuffer_ != nullptr) {
      xStreamBufferSend(streamBuffer_, data.data(), data.size(), 0);
    }
  }

  void flush() {
    if (streamBuffer_ != nullptr) {
      std::array<uint8_t, 64> txBuffer;

      // block until data is available in the stream buffer
      size_t bytesRead =
          xStreamBufferReceive(streamBuffer_, txBuffer.data(), txBuffer.size(), portMAX_DELAY);
      if (bytesRead > 0) {
        driver_.write(std::span(txBuffer.data(), bytesRead));
      }
    }
  }

 private:
  IUsbDriver& driver_;
  StreamBufferHandle_t streamBuffer_ = nullptr;
};

}  // namespace usb