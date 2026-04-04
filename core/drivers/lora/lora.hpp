#pragma once

#include <etl/vector.h>

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

class ILoraDriver {
 public:
  virtual ~ILoraDriver() = default;

  virtual void init(LoraConfig config) = 0;

  virtual void send(const uint8_t* data, size_t len) = 0;

  virtual void receive(uint8_t* buffer, size_t len) = 0;
};

class Lora {
 public:
  Lora(ILoraDriver& driver) : driver_(driver) {}
  ~Lora() = default;

  Lora(const Lora&) = delete;
  Lora& operator=(const Lora&) = delete;
  Lora(Lora&&) = delete;
  Lora& operator=(Lora&&) = delete;

  static constexpr size_t RADIO_FIFO_SIZE = 255;

 private:
  ILoraDriver& driver_;

  // buffer for storing data (ie. CAN frames) to be sent to the LoRa radio
  // size (255B) is max FIFO depth of RFM95, can be adjusted for other radios
  etl::vector<uint8_t, RADIO_FIFO_SIZE> dataBuffer_;
};

}  // namespace lora