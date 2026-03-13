#pragma once

#include "drivers/lora/lora.hpp"

namespace wireless {

class Wireless {
 public:
  Wireless(lora::Lora& lora) : lora_(lora) {}
  ~Wireless() = default;

  // delete copy and move
  Wireless(const Wireless&) = delete;
  Wireless& operator=(const Wireless&) = delete;
  Wireless(Wireless&&) = delete;
  Wireless& operator=(Wireless&&) = delete;

 private:
  lora::Lora& lora_;
};
}  // namespace wireless