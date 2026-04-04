#pragma once

#include <array>
#include <cstdint>
#include <span>

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

  void init() {
    hspi_.Instance = SPI2;
    hspi_.Init.Mode = SPI_MODE_MASTER;
    hspi_.Init.Direction = SPI_DIRECTION_2LINES;
    hspi_.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi_.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi_.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi_.Init.NSS = SPI_NSS_SOFT;
    hspi_.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi_.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi_.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi_.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi_.Init.CRCPolynomial = 10;

    HAL_SPI_Init(&hspi_);
  }

  void configureCs(GPIO_TypeDef* csPort, uint16_t csPin) {
    csPort_ = csPort;
    csPin_ = csPin;
  }

  void writeReg(uint8_t reg, uint8_t value) {
    std::array<uint8_t, 2> tx = {static_cast<uint8_t>(0x80 | reg), value};

    HAL_GPIO_WritePin(csPort_, csPin_, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi_, tx.data(), 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(csPort_, csPin_, GPIO_PIN_SET);
  }

  uint8_t readReg(uint8_t reg) {
    std::array<uint8_t, 2> tx = {static_cast<uint8_t>(0x7F & reg), 0x00};
    std::array<uint8_t, 2> rx = {0, 0};

    HAL_GPIO_WritePin(csPort_, csPin_, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi_, tx.data(), rx.data(), 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(csPort_, csPin_, GPIO_PIN_SET);

    return rx.at(1);
  }

  void burstWrite(uint8_t reg, std::span<const uint8_t> data) {
    std::array<uint8_t, 1> tx = {static_cast<uint8_t>(0x80 | reg)};

    HAL_GPIO_WritePin(csPort_, csPin_, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi_, tx.data(), tx.size(), HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi_, data.data(), data.size(), HAL_MAX_DELAY);
    HAL_GPIO_WritePin(csPort_, csPin_, GPIO_PIN_SET);
  }

 private:
  SPI_HandleTypeDef hspi_;

  GPIO_TypeDef* csPort_;
  uint16_t csPin_;
};

}  // namespace spi