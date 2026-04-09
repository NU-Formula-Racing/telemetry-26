#pragma once

#include <etl/vector.h>

#include <cstdint>
#include <span>

namespace lora {

enum class BoardType : uint8_t { BASE_STATION, REMOTE };
enum class LoraMode : uint8_t { SLEEP, STANDBY, TX, RX_CONTINUOUS };

// TODO: fill in other fields to make lib more configurable
struct LoraConfig {
  // mode (might need to be configurable so we can change between sleep, standby, tx, rx)
  // frequency = 915MHz
  // bandwidth
  // SF
  // tx power
  BoardType boardType;
};

struct RxPacket {
  std::span<const uint8_t> data;
  int16_t rssi;
  float snr;

  size_t size() const { return data.size(); }

  bool empty() const { return data.empty(); }

  std::string toString() const {
    std::string res = "rssi: " + std::to_string(rssi) + ", ";

    res += "snr: " + std::to_string(snr) + ", ";

    res += "data: [";
    for (const auto& byte : data) {
      res += std::to_string(byte) + " ";
    }
    res += "]";

    return res;
  }
};

class ILoraDriver {
 public:
  virtual ~ILoraDriver() = default;

  virtual void init(const LoraConfig& config) = 0;

  virtual void setMode(LoraMode mode) = 0;

  // true = sent, false = dropped
  virtual bool send(std::span<const uint8_t> data) = 0;

  virtual RxPacket receive() = 0;

  virtual bool isTransmitting() = 0;

  virtual bool packetWaiting() = 0;
};

class Lora {
 public:
  Lora(ILoraDriver& driver) : driver_(driver) {}
  ~Lora() = default;

  Lora(const Lora&) = delete;
  Lora& operator=(const Lora&) = delete;
  Lora(Lora&&) = delete;
  Lora& operator=(Lora&&) = delete;

  void init(const LoraConfig& config) { driver_.init(config); }

  void setMode(const LoraMode& mode) { driver_.setMode(mode); }

  bool isTransmitting() { return driver_.isTransmitting(); }

  bool send(std::span<const uint8_t> data) {
    if (data.size() > RADIO_FIFO_SIZE) {
      // packet is too big, dropping extra data for now
      // TODO: maybe implement fragmentation for larger packets if needed
      data = data.subspan(0, RADIO_FIFO_SIZE);
    }
    return driver_.send(data);
  }

  RxPacket receive() { return driver_.receive(); }

  // size (255B) is max FIFO depth of RFM95, can be adjusted for other radios
  static constexpr size_t RADIO_FIFO_SIZE = 255;

 private:
  ILoraDriver& driver_;
};

}  // namespace lora