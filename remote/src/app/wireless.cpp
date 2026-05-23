#include "wireless.hpp"

namespace wireless {

ProtocolState StateUnconnected::react(const EvtTick& evt, Wireless& ctx) {
  DEBUG_OUT("WIRELESS", MAGENTA, "Unconnected, sending handshake req at time ",
            std::to_string(evt.currentTimeMs), "\r\n");
  ctx.sendHandshakeReq();
  return StateHandshakePending{evt.currentTimeMs};
}

ProtocolState StateHandshakePending::react(const EvtTick& evt, Wireless& /*ctx*/) {
  DEBUG_OUT("WIRELESS", MAGENTA, "Handshake pending, checking for timeout at time ",
            std::to_string(evt.currentTimeMs), "\r\n");
  if (evt.currentTimeMs - lastTxTimeMs > HANDSHAKE_TIMEOUT_MS) {
    // timeout
    DEBUG_OUT("WIRELESS", MAGENTA, "Handshake timeout, returning to unconnected state\r\n");
    return StateUnconnected{};
  }
  // lastTxTimeMs = evt.currentTimeMs;
  // return StateHandshakePending{evt.currentTimeMs};
  return *this;
}

ProtocolState StateHandshakePending::react(const EvtRxAck& /*evt*/, Wireless& /*ctx*/) {
  DEBUG_OUT("WIRELESS", MAGENTA, "Handshake ack received, transitioning to connected state\r\n");
  // something maybe to indicate we just connected
  return StateConnected{};
}

ProtocolState StateConnected::react(const EvtTick& /*evt*/, Wireless& ctx) {
  DEBUG_OUT("WIRELESS", MAGENTA, "Connected, sending can frames\r\n");
  ctx.sendCanFrames();
  return StateConnected{};
}

}  // namespace wireless