#pragma once

#include <FreeRTOS.h>
#include <queue.h>

#include <array>
#include <cstdint>
#include <format>
#include <string>

#include "can_types.hpp"

namespace can {

enum class CanStatus : uint8_t { OK, ERROR };

// callback type for RX interrupt
using CanRxCallback = void (*)(const CanFrame& frame);

class ICanDriver {
 public:
  virtual ~ICanDriver() = default;

  virtual CanStatus init() = 0;

  virtual CanStatus send(const CanFrame& frame) = 0;

  virtual void registerRxCallback(CanRxCallback cb) = 0;
};

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