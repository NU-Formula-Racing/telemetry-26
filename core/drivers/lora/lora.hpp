#pragma once

#include <cstdint>

namespace lora {

enum class BoardType : uint8_t { BASE_STATION, REMOTE };

struct LoraConfig {
  // mode (might need to be configurable so we can change between sleep, standby, tx, rx)
  // frequency = 915MHz
  // bandwidth
  // SF
  // tx power
  BoardType boardType;
};

class ILora {
 public:
  virtual ~ILora() = default;

  virtual void init(LoraConfig config) = 0;

  virtual void send(const uint8_t* data, size_t len) = 0;

  virtual void receive(uint8_t* buffer, size_t len) = 0;
};

}  // namespace lora