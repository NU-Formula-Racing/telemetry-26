#pragma once

#include <cstring>
#include <variant>

#include "lora.hpp"
#include "packet_types.hpp"
#include "projdefs.h"
#include "stm32f4xx_hal.h"
#include "tasks/job.hpp"
#include "utils/utils.hpp"
#include "wireless.hpp"

namespace base {

// protocol events
struct EvtTick {
  uint32_t currentTimeMs;
};
struct EvtRxHandshakeReq {
  uint32_t rxTimeMs;
  lora::RxPacket handshakeReqPacket;  // dataPacket.data is an entire DataPacket
};
struct EvtRxData {
  uint32_t rxTimeMs;
  lora::RxPacket dataPacket;  // dataPacket.data is an entire DataPacket
};
using ProtocolEvent = std::variant<EvtTick, EvtRxHandshakeReq, EvtRxData>;

// base station forward declaration
class BaseStation;

// protocol states
struct StateUnconnected;
struct StateConnected;
using ProtocolState = std::variant<StateUnconnected, StateConnected>;

// protocol state definitions
struct StateUnconnected {
  ProtocolState react(const EvtTick& evt, BaseStation& ctx);
  ProtocolState react(const EvtRxHandshakeReq& evt, BaseStation& ctx);
  template <typename T>
  ProtocolState react(const T& /*unused*/, BaseStation& /*unused*/);
};

struct StateConnected {
  uint32_t lastRxTimeMs;
  static constexpr uint32_t DATA_TIMEOUT_MS = 5000;

  ProtocolState react(const EvtTick& evt, BaseStation& ctx);
  ProtocolState react(const EvtRxHandshakeReq& evt, BaseStation& ctx);
  ProtocolState react(const EvtRxData& evt, BaseStation& ctx);
  template <typename T>
  ProtocolState react(const T& /*unused*/, BaseStation& /*unused*/);
};

// default template methods ned to be definted after structs but still in header
template <typename T>
ProtocolState StateUnconnected::react(const T& /*unused*/, BaseStation& /*unused*/) {
  return *this;
}
template <typename T>
ProtocolState StateConnected::react(const T& /*unused*/, BaseStation& /*unused*/) {
  return *this;
}

class BaseStation {
 public:
  BaseStation(wireless::Wireless& wireless /*, Usb& usb*/)
      : wireless_(wireless) /*, usb_(usb)*/, protocolState_(StateUnconnected{}) {}
  ~BaseStation() = default;

  BaseStation(const BaseStation&) = delete;
  BaseStation& operator=(const BaseStation&) = delete;
  BaseStation(BaseStation&&) = delete;
  BaseStation& operator=(BaseStation&&) = delete;

  void init() {
    wireless_.init();
    // usb_.init();
  }

  void processIncomingPackets() {
    auto packet = wireless_.receive();
    if (packet.empty()) {
      return;
    }

    // usb_.write(packet);
    DEBUG_OUT("BaseStation", CYAN, "Received packet of size ", std::to_string(packet.size()), ": ",
              packet.toString(), "\r\n");
  }

  void sendHandshakeRes(uint32_t sessionId) {
    protocol::PacketHeader header{};
    header.magic = protocol::MAGIC;
    header.sessionId = sessionId;
    header.type = protocol::PacketType::HANDSHAKE_ACK;
    header.length = 0;

    std::array<uint8_t, sizeof(protocol::PacketHeader)> packet;
    std::memcpy(packet.data(), &header, sizeof(protocol::PacketHeader));

    DEBUG_OUT("WIRELESS", CYAN, "Sending handshake res with session ID ", std::to_string(sessionId),
              "\r\n");

    wireless_.send(packet);
  }

  void streamOverUsb(const lora::RxPacket& /*packet*/) {
    // usb_.write(packet);
  }

  void processEvent(const ProtocolEvent& evt) {
    // DEBUG_OUT("WIRELESS", YELLOW, "Processing event of type ", std::to_string(evt.index()),
    //           " in state ", std::to_string(protocolState_.index()), "\r\n");
    ProtocolState nextState = std::visit(
        [this](auto& state, const auto& ev) -> ProtocolState { return state.react(ev, *this); },
        protocolState_, evt);
    // if a new state was returned, transition
    protocolState_ = nextState;
  }

  void update(const uint32_t currentTimeMs) {
    // DEBUG_OUT("BaseStation", YELLOW,
    //           "top of update, current state: ", std::to_string(protocolState_.index()), "\r\n");
    EvtTick evt{};
    evt.currentTimeMs = currentTimeMs;
    processEvent(evt);

    if (!wireless_.isTransmitting()) {
      wireless_.setMode(lora::LoraMode::RX_CONTINUOUS);
    }

    auto rxPacket = wireless_.receive();
    if (!rxPacket.empty()) {
      DEBUG_OUT("BaseStation", CYAN, "Received packet of size ", std::to_string(rxPacket.size()),
                ": ", rxPacket.toString(), "\r\n");
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
        if (header.type == protocol::PacketType::HANDSHAKE_REQ) {
          DEBUG_OUT("BaseStation", CYAN, "Received handshake req with session ID ",
                    std::to_string(header.sessionId), "\r\n");
          EvtRxHandshakeReq handshakeReqEvt{};
          handshakeReqEvt.rxTimeMs = currentTimeMs;
          handshakeReqEvt.handshakeReqPacket = rxPacket;
          processEvent(handshakeReqEvt);
        } else if (header.type == protocol::PacketType::DATA) {
          DEBUG_OUT("BaseStation", CYAN, "Received data packet with session ID ",
                    std::to_string(header.sessionId), "and length ", std::to_string(header.length),
                    "\r\n");
          EvtRxData dataEvt{};
          dataEvt.rxTimeMs = currentTimeMs;
          dataEvt.dataPacket = rxPacket;
          processEvent(dataEvt);
        }
      }
    } else {
      DEBUG_OUT("BaseStation", CYAN, "No packet received\r\n");
    }
  }

 private:
  // Usb& usb_;
  wireless::Wireless& wireless_;
  ProtocolState protocolState_;
};

class WirelessUpdateJob : public tasks::IJob {
 public:
  WirelessUpdateJob(BaseStation& baseStation) : baseStation_(baseStation) {}
  ~WirelessUpdateJob() override = default;

  // delete copy and move
  WirelessUpdateJob(const WirelessUpdateJob&) = delete;
  WirelessUpdateJob& operator=(const WirelessUpdateJob&) = delete;
  WirelessUpdateJob(WirelessUpdateJob&&) = delete;
  WirelessUpdateJob& operator=(WirelessUpdateJob&&) = delete;

  void init() override { baseStation_.init(); }

  void run() override { baseStation_.update(HAL_GetTick()); }

 private:
  BaseStation& baseStation_;
};

}  // namespace base