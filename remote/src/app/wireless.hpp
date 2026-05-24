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
#include "packet_types.hpp"
#include "stm32f4xx_hal.h"
#include "tasks/job.hpp"
#include "utils/utils.hpp"

namespace wireless {

#pragma pack(push, 1)
struct WirelessFrame {
  can::CanFrame canFrame;  // CAN data
  bool dirty;  // flag to indicate if this frame contains new data (not yet sent to the LoRa radio)
};
#pragma pack(pop)

// protocol events
struct EvtTick {
  uint32_t currentTimeMs;
};
struct EvtRxAck {
  uint32_t sessionId;
};
struct EvtRxData {};
using ProtocolEvent = std::variant<EvtTick, EvtRxAck, EvtRxData>;

// wireless forward declaration
class Wireless;

// protocol states
struct StateUnconnected;
struct StateHandshakePending;
struct StateConnected;
using ProtocolState = std::variant<StateUnconnected, StateHandshakePending, StateConnected>;

// protocol state definitions
struct StateUnconnected {
  ProtocolState react(const EvtTick& evt, Wireless& ctx);
  template <typename T>
  ProtocolState react(const T& /*unused*/, Wireless& /*unused*/);
};

struct StateHandshakePending {
  uint32_t lastTxTimeMs = 0;
  static constexpr uint32_t HANDSHAKE_TIMEOUT_MS = 5000;

  ProtocolState react(const EvtTick& evt, Wireless& ctx);
  ProtocolState react(const EvtRxAck& evt, Wireless& ctx);
  template <typename T>
  ProtocolState react(const T& /*unused*/, Wireless& /*unused*/);
};

struct StateConnected {
  ProtocolState react(const EvtTick& evt, Wireless& ctx);
  template <typename T>
  ProtocolState react(const T& /*unused*/, Wireless& /*unused*/);
};

// protocol default template methods need to be defined after the structs but still in the header
template <typename T>
ProtocolState StateUnconnected::react(const T& /*unused*/, Wireless& /*unused*/) {
  return *this;
}
template <typename T>
ProtocolState StateHandshakePending::react(const T& /*unused*/, Wireless& /*unused*/) {
  return *this;
}
template <typename T>
ProtocolState StateConnected::react(const T& /*unused*/, Wireless& /*unused*/) {
  return *this;
}

class Wireless {
 public:
  Wireless(lora::Lora& lora) : lora_(lora), protocolState_(StateUnconnected{}) {}
  ~Wireless() = default;

  // delete copy and move
  Wireless(const Wireless&) = delete;
  Wireless& operator=(const Wireless&) = delete;
  Wireless(Wireless&&) = delete;
  Wireless& operator=(Wireless&&) = delete;

  void init() {
    txIndex_ = 0;
    canDataBuffer_.clear();
    lora_.init(config_);
  }

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

  bool sendCanFrames() {
    // if (canDataBuffer_.empty() || lora_.isTransmitting()) {
    //   return false;
    // }

    if (lora_.isTransmitting()) {
      DEBUG_OUT("WIRELESS", YELLOW, "LoRa is currently transmitting, skipping send\r\n");
      return false;
      // TODO: return a diff status, being busy is normal and fine and shouldnt be false
    }

    if (canDataBuffer_.empty()) {
      // std::string emptyMsg = "hello workld\r\n";
      // lora_.send(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(emptyMsg.data()),
      //                                     emptyMsg.size()));
      return true;
    }

    // etl::vector<uint8_t, lora::Lora::RADIO_FIFO_SIZE> packet{};

    protocol::DataPacket packet{};
    packet.header.magic = protocol::MAGIC;
    packet.header.sessionId = sessionId_;
    packet.header.type = protocol::PacketType::DATA;
    packet.header.length = 0;

    const size_t totalFrames = canDataBuffer_.size();
    size_t framesChecked = 0;

    if (xSemaphoreTake(mutex_, portMAX_DELAY) == pdTRUE) {
      while (framesChecked < totalFrames) {
        WirelessFrame& wf = canDataBuffer_.at(txIndex_);
        if (wf.dirty) {
          if (packet.payload.size() + sizeof(can::CanFrame) > packet.payload.max_size()) {
            break;  // packet cant fit any more frames
          }

          auto frameBytes = std::bit_cast<std::array<uint8_t, sizeof(can::CanFrame)>>(wf.canFrame);
          packet.payload.insert(packet.payload.end(), frameBytes.begin(), frameBytes.end());
          packet.header.length += sizeof(can::CanFrame);

          wf.dirty = false;
        }
        txIndex_ = (txIndex_ + 1) % totalFrames;
        framesChecked++;
      }
      xSemaphoreGive(mutex_);
    }

    if (!packet.payload.empty()) {
      // DEBUG_OUT("WIRELESS", GREEN, "Sending packet of size ", std::to_string(packet.size()),
      //           "\r\n");

      std::array<uint8_t, sizeof(protocol::DataPacket)> packetBytes{};
      std::memcpy(packetBytes.data(), &packet, sizeof(protocol::DataPacket));
      lora_.send(packetBytes);
    }

    return false;
  }

  lora::HardwareStatus getHardwareStatus() { return lora_.getHardwareStatus(); }

  uint8_t getProtocolState() const { return static_cast<uint8_t>(protocolState_.index()); }

  bool isConnected() const { return std::holds_alternative<StateConnected>(protocolState_); }

  void processEvent(const ProtocolEvent& evt) {
    DEBUG_OUT("WIRELESS", BLUE, "Processing event of type ", std::to_string(evt.index()),
              " in state ", std::to_string(protocolState_.index()), "\r\n");
    ProtocolState nextState = std::visit(
        [this](auto& state, const auto& ev) -> ProtocolState { return state.react(ev, *this); },
        protocolState_, evt);
    // if a new state was returned, transition
    protocolState_ = nextState;
  }

  void update(uint32_t currentTimeMs) {
    DEBUG_OUT("WIRELESS", BLUE,
              "top of update, current state: ", std::to_string(protocolState_.index()), "\r\n");
    EvtTick evt{};
    evt.currentTimeMs = currentTimeMs;
    processEvent(evt);

    if (!isConnected()) {
      DEBUG_OUT("WIRELESS", BLUE, "Not connected, checking for incoming packets\r\n");
      auto rxPacket = lora_.receive();
      if (!rxPacket.empty()) {
        if (rxPacket.size() >= sizeof(protocol::PacketHeader)) {
          // decode header
          protocol::PacketHeader header{};
          std::memcpy(&header, rxPacket.data.data(), sizeof(protocol::PacketHeader));

          // check header.magic matches
          if (header.magic != protocol::MAGIC) {
            DEBUG_OUT("WIRELESS", RED, "Received packet with invalid magic, dropping\r\n");
            return;
          }

          // translate raw packet into an fsm event
          if (header.type == protocol::PacketType::HANDSHAKE_ACK) {
            EvtRxAck ackEvt{};
            ackEvt.sessionId = header.sessionId;
            processEvent(ackEvt);
          }
        }
      }
    }
  }

  void sendHandshakeReq() {
    // generate random session ID
    sessionId_ = static_cast<uint32_t>(rand());

    DEBUG_OUT("WIRELESS", CYAN, "Sending handshake req with session ID ",
              std::to_string(sessionId_), "\r\n");

    protocol::PacketHeader header{};
    header.sessionId = sessionId_;
    header.type = protocol::PacketType::HANDSHAKE_REQ;
    header.length = 0;

    std::array<uint8_t, sizeof(protocol::PacketHeader)> packet;
    std::memcpy(packet.data(), &header, sizeof(protocol::PacketHeader));

    lora_.send(packet);
  }

 private:
  lora::Lora& lora_;

  ProtocolState protocolState_;
  uint32_t sessionId_ = 0;

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

class WirelessUpdateJob : public tasks::IJob {
 public:
  WirelessUpdateJob(Wireless& wireless) : wireless_(wireless) {}
  ~WirelessUpdateJob() override = default;

  // delete copy and move
  WirelessUpdateJob(const WirelessUpdateJob&) = delete;
  WirelessUpdateJob& operator=(const WirelessUpdateJob&) = delete;
  WirelessUpdateJob(WirelessUpdateJob&&) = delete;
  WirelessUpdateJob& operator=(WirelessUpdateJob&&) = delete;

  void init() override {}

  void run() override {
    // DEBUG_OUT("WirelessUpdateJob", BLUE, "Updating wireless...\r\n");
    //  bool res = wireless_.sendCanFrames();
    wireless_.update(HAL_GetTick());
    // if (!res) {
    //   ERROR("LoraWriteJob", "LoRa packet dropped or empty :(\r\n");
    // } else {
    //   // DEBUG_OUT("LoraWriteJob", CYAN, "LoRa packet sent successfully :)\r\n");
    // }
  }

 private:
  Wireless& wireless_;
};

}  // namespace wireless