#pragma once

#include "spi.hpp"

namespace lora {

class Lora {
 public:
  Lora(spi::Spi& spi) : spi_(spi) {}
  ~Lora() = default;

  // delete copy and move
  Lora(const Lora&) = delete;
  Lora& operator=(const Lora&) = delete;
  Lora(Lora&&) = delete;
  Lora& operator=(Lora&&) = delete;

 private:
  spi::Spi& spi_;
};

}  // namespace lora