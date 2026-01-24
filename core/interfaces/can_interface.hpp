#pragma once

#include <cstdint>

namespace can {

// required can functionality:
// init (configure baud rate, automatic bus-off management, etc.)
// send (oneshot, periodic)
// recieve (oneshot, periodic)
// TODO: implement support for big + little endian

// @brief Supported CAN Baud Rates (in bps)
enum class canBaudRate : uint8_t { BAUD_125K, BAUD_250K, BAUD_500K, BAUD_1M };

struct CanSignal {
  uint8_t startBit;
  uint8_t length;
  bool
};

struct CanMessage {
  uint32_t id;

  etl::vector<CanSignal, 16> signals;
};

class ICan {
 public:
  ICan(canBaudRate baudRate) : baudRate_(baudRate) {};
  bool init();
  bool send(const CanMessage& message);
  bool receive(CanMessage& message);

 private:
  canBaudRate baudRate_;
};

}  // namespace can