#pragma once

#include <FreeRTOS.h>
#include <etl/vector.h>
#include <semphr.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <span>

#include "can.hpp"
#include "drivers/lora/lora.hpp"
#include "job.hpp"

namespace wireless {

#pragma pack(push, 1)
struct WirelessFrame {
  can::CanFrame canFrame;  // CAN data
  bool dirty;  // flag to indicate if this frame contains new data (not yet sent to the LoRa radio)
};
#pragma pack(pop)

class Wireless {
 public:
  Wireless(lora::Lora& lora) : lora_(lora) {}
  ~Wireless() = default;

  // delete copy and move
  Wireless(const Wireless&) = delete;
  Wireless& operator=(const Wireless&) = delete;
  Wireless(Wireless&&) = delete;
  Wireless& operator=(Wireless&&) = delete;

  void init() { lora_.init(config_); }

  void updateCanFrame(const can::CanFrame& frame) {
    // we dont want to wirelessly transmit extended ID msgs (DBC msgs only use std IDs)
    // extended IDs are used for stuff we dgaf about (ie. VESC flashing over CAN for inverters)
    if (frame.idType == can::CanIdType::EXTENDED) {
      return;
    }

    if (xSemaphoreTake(mutex_, portMAX_DELAY) == pdTRUE) {
      // check if frame with same ID already exists in buffer
      auto* it = std::ranges::find_if(
          canDataBuffer_, [&frame](const WirelessFrame& wf) { return wf.canFrame.id == frame.id; });

      if (it != canDataBuffer_.end()) {
        // frame with same ID already exists, update it
        it->canFrame = frame;
        it->dirty = true;
      } else {
        // frame with same ID doesnt exist, add new one to buffer
        // drop if buffer is full
        if (!canDataBuffer_.full()) {
          canDataBuffer_.push_back({.canFrame = frame, .dirty = true});
        }
      }

      xSemaphoreGive(mutex_);
    }
  }

  void sendCanFrames() {
    if (canDataBuffer_.empty()) {
      return;
    }

    etl::vector<uint8_t, lora::Lora::RADIO_FIFO_SIZE> packet{};
    const size_t totalFrames = canDataBuffer_.size();
    size_t framesChecked = 0;

    if (xSemaphoreTake(mutex_, portMAX_DELAY) == pdTRUE) {
      while (framesChecked < totalFrames) {
        WirelessFrame& wf = canDataBuffer_.at(txIndex_);
        if (wf.dirty) {
          if (packet.size() + sizeof(can::CanFrame) > packet.max_size()) {
            break;  // packet cant fit any more frames
          }

          auto frameBytes = std::bit_cast<std::array<uint8_t, sizeof(can::CanFrame)>>(wf.canFrame);
          packet.insert(packet.end(), frameBytes.begin(), frameBytes.end());

          wf.dirty = false;
        }
        txIndex_ = (txIndex_ + 1) % totalFrames;
        framesChecked++;
      }
      xSemaphoreGive(mutex_);
    }

    if (!packet.empty()) {
      lora_.send(std::span(packet.data(), packet.size()));
    }
  }

 private:
  lora::Lora& lora_;

  // buffer for storing CAN frame data
  // broken into packets and sent to the LoRa radio periodically
  // size is 120 to account for ~100 CAN IDs in DBC (+ some extra room just in case)
  static constexpr size_t CAN_BUFFER_SIZE = 120;
  etl::vector<WirelessFrame, CAN_BUFFER_SIZE> canDataBuffer_;

  // index for tracking where we are in the data buffer when sending
  size_t txIndex_ = 0;

  SemaphoreHandle_t mutex_ = xSemaphoreCreateMutex();

  lora::LoraConfig config_{
      .boardType = lora::BoardType::REMOTE,
  };
};

class LoraWriteJob : public tasks::IJob {
 public:
  LoraWriteJob(Wireless& wireless) : wireless_(wireless) {}
  ~LoraWriteJob() override = default;

  // delete copy and move
  LoraWriteJob(const LoraWriteJob&) = delete;
  LoraWriteJob& operator=(const LoraWriteJob&) = delete;
  LoraWriteJob(LoraWriteJob&&) = delete;
  LoraWriteJob& operator=(LoraWriteJob&&) = delete;

  void init() override {}

  void run() override { wireless_.sendCanFrames(); }

 private:
  Wireless& wireless_;
};

}  // namespace wireless