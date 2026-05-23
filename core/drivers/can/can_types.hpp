#pragma once

#include <cstdint>
#include <cstring>
#include <format>
#include <string>

namespace can {

constexpr uint32_t STD_ID_MASK = 0x7FF;
constexpr uint32_t EXT_ID_MASK = 0x1FFFFFFF;

enum class CanBaudRate : uint8_t { BAUD_125K, BAUD_250K, BAUD_500K, BAUD_1M };
enum class CanIdType : uint8_t { STANDARD = 0x00, EXTENDED = 0x04 };
enum class CanEndianness : uint8_t { LITTLE, BIG };

#pragma pack(push, 1)
// raw can frame
struct CanFrame {
  uint32_t timestamp;           // in ms, HAL_GetTick()
  uint32_t id;                  // 11 bit standard ID or 29 bit extended ID
  uint8_t dlc;                  // data length in bytes (0-8)
  CanIdType idType;             // standard or extended
  std::array<uint8_t, 8> data;  // data payload (0-8 bytes)

  std::string dataToString() const {
    // apple clang doesnt support std::format :(
    // return std::format("{:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X}", data.at(0),
    //                   data.at(1), data.at(2), data.at(3), data.at(4), data.at(5), data.at(6),
    //                   data.at(7));

    std::array<char, 32> buf{};
    std::snprintf(buf.data(), buf.size(), "%02X %02X %02X %02X %02X %02X %02X %02X", data.at(0),
                  data.at(1), data.at(2), data.at(3), data.at(4), data.at(5), data.at(6),
                  data.at(7));
    return std::string(buf.data());
  }
};
#pragma pack(pop)

template <typename T>
CanFrame encode(const T& msg) {
  static_assert(sizeof(T) <= sizeof(CanFrame::data),
                "Message size exceeds max CAN frame data length of 8 bytes");

  CanFrame frame{};
  frame.id = T::ID;
  frame.idType = T::ID_TYPE;
  frame.dlc = static_cast<uint8_t>(sizeof(T));
  std::memcpy(frame.data.data(), &msg, sizeof(T));

  return frame;
}

template <typename T>
T decode(const CanFrame& frame) {
  T msg{};
  const size_t numBytesToCopy = std::min(static_cast<size_t>(frame.dlc), sizeof(T));
  std::memcpy(&msg, frame.data.data(), numBytesToCopy);

  return msg;
}

}  // namespace can