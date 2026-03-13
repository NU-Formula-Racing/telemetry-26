#pragma once

#include "stm32f405xx.h"
#include "stm32f4xx_hal_spi.h"

namespace spi {

class Spi {
 public:
  Spi() = default;
  ~Spi() = default;

  // delete copy and move
  Spi(const Spi&) = delete;
  Spi& operator=(const Spi&) = delete;
  Spi(Spi&&) = delete;
  Spi& operator=(Spi&&) = delete;

 private:
  SPI_HandleTypeDef hspi_;
};

}  // namespace spi