#include <cstddef>

#include "interface.hpp"

// struct LogHeader {
//   std::array<uint8_t, 9> version;  // NFR26000\n -> NFR26 v0.0.0
//   uint32_t startTimestamp;         // RTC value, idk if its actually uint32_t
//   // TODO: gps
// };
//

// align to 1 byte to avoid padding
// create in CAN ISR
// #pragma pack(push, 1)
// struct LogFrame {
//   uint32_t timestamp;  // offset from start in ms
//   // TODO: gps
//   uint32_t id;  // 12 bit standard ID or 29 bit extended ID
//   uint8_t len;  // data length in bits
//   std::array<uint8_t, 8> data;
// };
// #pragma pack(pop)

class App {
 private:
 public:
  App() = default;
  int run();
};