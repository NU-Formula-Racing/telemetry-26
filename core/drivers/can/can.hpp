#pragma once

#include <FreeRTOS.h>
#include <queue.h>

#include <array>
#include <cstdint>
#include <format>
#include <string>

namespace can {

enum class CanBaudRate : uint8_t { BAUD_125K, BAUD_250K, BAUD_500K, BAUD_1M };
enum class CanIdType : uint8_t { STANDARD = 0x00, EXTENDED = 0x04 };
enum class CanEndianess : uint8_t { LITTLE, BIG };  // TODO

enum class CanStatus : uint8_t { OK, ERROR };

#pragma pack(push, 1)
// raw can frame
struct CanFrame {
  uint32_t timestamp;           // in ms, HAL_GetTick()
  uint32_t id;                  // 11 bit standard ID or 29 bit extended ID
  uint8_t dlc;                  // data length in bytes (0-8)
  CanIdType idType;             // standard or extended
  std::array<uint8_t, 8> data;  // data payload (0-8 bytes)

  std::string dataToString() const {
    return std::format("{:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X}", data.at(0),
                       data.at(1), data.at(2), data.at(3), data.at(4), data.at(5), data.at(6),
                       data.at(7));
  }
};
#pragma pack(pop)

// callback type for RX interrupt
using CanRxCallback = void (*)(const CanFrame& frame);

// for decoding signals from raw CAN frames
struct CanSignal {
  uint8_t startBit;
  uint8_t length;  // in bits
  bool isSigned;
  float scale;
  float offset;
};

// TODO: decoded CAN message with signals

class ICanDriver {
 public:
  virtual ~ICanDriver() = default;

  virtual CanStatus init() = 0;

  virtual CanStatus send(const CanFrame& frame) = 0;

  virtual void registerRxCallback(CanRxCallback cb) = 0;
};

// TODO: decode/encode messages, abstract raw frame from CanBus layer, store and apply DBC info,
class CanBus {
 public:
  CanBus(ICanDriver& driver) : driver_(driver) {
    rxQueue_ = xQueueCreate(queueLength_, sizeof(CanFrame));
  }
  ~CanBus() = default;

  CanBus(const CanBus&) = delete;
  CanBus& operator=(const CanBus&) = delete;
  CanBus(CanBus&&) = delete;
  CanBus& operator=(CanBus&&) = delete;

  CanStatus init() {
    driver_.registerRxCallback(isrToQueueRouter);
    return driver_.init();
  }

  bool receive(CanFrame& outFrame, TickType_t timeout = portMAX_DELAY) {
    // get next frame from queue, wait indefinitely by default
    auto status = xQueueReceive(rxQueue_, &outFrame, timeout) == pdTRUE;

    return status;
  }

  CanStatus send(const CanFrame& frame) { return driver_.send(frame); }

 private:
  static void isrToQueueRouter(const CanFrame& frame) {
    if (rxQueue_ == nullptr) {
      return;  // queue not initialized, drop frame
    }

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // push new frame to queue
    // to avoid blocking in ISR, if queue is full, new frame will be dropped
    xQueueSendFromISR(rxQueue_, &frame, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }

  ICanDriver& driver_;
  inline static QueueHandle_t rxQueue_ = nullptr;
  static constexpr size_t queueLength_ = 500;
};

}  // namespace can