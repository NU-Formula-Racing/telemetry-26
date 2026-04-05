#pragma once

#include "lora/lora.hpp"

namespace wireless {

class Wireless {
 public:
  Wireless(lora::Lora& lora) : lora_(lora) {}
  ~Wireless() = default;

  Wireless(const Wireless&) = delete;
  Wireless& operator=(const Wireless&) = delete;
  Wireless(Wireless&&) = delete;
  Wireless& operator=(Wireless&&) = delete;

  void init() { lora_.init(config_); }

  // receive() {}

 private:
  lora::Lora& lora_;

  lora::LoraConfig config_{
      .boardType = lora::BoardType::BASE_STATION,
  };
};

}  // namespace wireless