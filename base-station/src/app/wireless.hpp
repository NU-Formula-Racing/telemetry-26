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

  std::span<const uint8_t> receive() { return lora_.receive(); }

 private:
  lora::Lora& lora_;
  // ProtocolHandler protocol_; // protocol fsm lives here

  lora::LoraConfig config_{
      .boardType = lora::BoardType::BASE_STATION,
  };
};

}  // namespace wireless