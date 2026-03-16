#pragma once

#include "drivers/lora/lora.hpp"

namespace wireless {

class Wireless {
 public:
  Wireless(lora::ILora& lora) : lora_(lora) {}
  ~Wireless() = default;

  // delete copy and move
  Wireless(const Wireless&) = delete;
  Wireless& operator=(const Wireless&) = delete;
  Wireless(Wireless&&) = delete;
  Wireless& operator=(Wireless&&) = delete;

  void init() { lora_.init(config_); }

 private:
  lora::ILora& lora_;

  lora::LoraConfig config_{
      .boardType = lora::BoardType::REMOTE,
  };
};
}  // namespace wireless