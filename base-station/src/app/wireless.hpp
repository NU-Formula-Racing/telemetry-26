#pragma once

#include "lora.hpp"

namespace wireless {

class Wireless {
 public:
  Wireless(lora::Lora& lora) : lora_(lora) {}
  ~Wireless() = default;

  Wireless(const Wireless&) = delete;
  Wireless& operator=(const Wireless&) = delete;
  Wireless(Wireless&&) = delete;
  Wireless& operator=(Wireless&&) = delete;

  void init() {
    lora_.init(config_);
    lora_.setMode(lora::LoraMode::RX_CONTINUOUS);
  }

  lora::RxPacket receive() { return lora_.receive(); }

  bool send(std::span<const uint8_t> data) { return lora_.send(data); }

  bool isTransmitting() { return lora_.isTransmitting(); }

  void setMode(lora::LoraMode mode) { lora_.setMode(mode); }

 private:
  lora::Lora& lora_;

  lora::LoraConfig config_{
      .boardType = lora::BoardType::BASE_STATION,
  };
};

}  // namespace wireless