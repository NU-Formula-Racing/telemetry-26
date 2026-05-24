#include "base_station.hpp"

#include "packet_types.hpp"

namespace base {

ProtocolState StateUnconnected::react(const EvtTick& /*evt*/, BaseStation& /*ctx*/) {
  // no comms from remote, stay in unconnected state
  // DEBUG_OUT("WIRELESS", MAGENTA, "Unconnected, waiting for handshake req\r\n");
  return *this;
}

ProtocolState StateUnconnected::react(const EvtRxHandshakeReq& evt, BaseStation& ctx) {
  // just received a handshake req
  // send handshake ack
  // transition to handshake pending state, start timer
  // also tell interface a new session is starting with this sessionID

  protocol::PacketHeader reqHeader{};
  std::memcpy(&reqHeader, evt.handshakeReqPacket.data.data(), sizeof(protocol::PacketHeader));

  DEBUG_OUT("WIRELESS", MAGENTA, "Received handshake req with session ID ",
            std::to_string(reqHeader.sessionId), ", sending handshake res\r\n");

  ctx.sendHandshakeRes(reqHeader.sessionId);
  ctx.streamOverUsb(evt.handshakeReqPacket);
  return StateConnected{.lastRxTimeMs = evt.rxTimeMs};
}

ProtocolState StateConnected::react(const EvtTick& evt, BaseStation& /*ctx*/) {
  if (evt.currentTimeMs - lastRxTimeMs > DATA_TIMEOUT_MS) {
    // timeout, transition back to unconnected
    DEBUG_OUT("WIRELESS", MAGENTA, "Data timeout, transitioning back to unconnected state\r\n");
    return StateUnconnected{};
  }
  return *this;
}

ProtocolState StateConnected::react(const EvtRxHandshakeReq& evt, BaseStation& ctx) {
  // remote restarted or something went wrong
  // we can just respond with a handshake ack and stay in connected state
  lastRxTimeMs = evt.rxTimeMs;

  protocol::PacketHeader reqHeader{};
  std::memcpy(&reqHeader, evt.handshakeReqPacket.data.data(), sizeof(protocol::PacketHeader));

  ctx.sendHandshakeRes(reqHeader.sessionId);
  ctx.streamOverUsb(evt.handshakeReqPacket);
  return *this;
}

ProtocolState StateConnected::react(const EvtRxData& evt, BaseStation& ctx) {
  // process: place into usb streambuffer
  lastRxTimeMs = evt.rxTimeMs;

  DEBUG_OUT("WIRELESS", CYAN, "Received data packet at time ", std::to_string(evt.rxTimeMs),
            ", and length ", std::to_string(evt.dataPacket.size()), "\r\n");

  ctx.streamOverUsb(evt.dataPacket);
  return *this;
}

};  // namespace base