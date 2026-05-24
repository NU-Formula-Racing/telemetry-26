#pragma once

#include <cstdint>

namespace protocol {

enum class PacketType : uint8_t { HANDSHAKE_REQ, HANDSHAKE_ACK, DATA };

#pragma pack(push, 1)
struct PacketHeader {
  uint8_t magic;  // fixed value to identify start of packet
  // uint8_t address;
  PacketType type;
  uint16_t length;  // length of payload in bytes
  // uint32_t seqNum; // sequence number
};
#pragma pack(pop)

};  // namespace protocol