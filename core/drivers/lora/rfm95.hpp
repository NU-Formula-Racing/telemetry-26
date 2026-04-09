#pragma once

#include <FreeRTOS.h>
#include <task.h>

#include <cstdint>

#include "lora.hpp"
#include "rfm95_utils.hpp"
#include "spi.hpp"
#include "tasks/job.hpp"
#include "utils/utils.hpp"

namespace lora::rfm95 {

class Rfm95 : public ILoraDriver {
 public:
  Rfm95() = default;
  ~Rfm95() = default;

  // delete copy and move
  Rfm95(const Rfm95&) = delete;
  Rfm95& operator=(const Rfm95&) = delete;
  Rfm95(Rfm95&&) = delete;
  Rfm95& operator=(Rfm95&&) = delete;

  void init(const LoraConfig& config) override {
    rxBuffer_.fill(0);

    setupSpi(config.boardType);

    // reset rfm95 module
    HAL_GPIO_WritePin(LORA_RESET_GPIO_Port, LORA_RESET_Pin, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(10));
    HAL_GPIO_WritePin(LORA_RESET_GPIO_Port, LORA_RESET_Pin, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(10));

    uint8_t version = spi_.readReg(REG_VERSION);
    if (version != 0x12) {
      // rfm95 module not detected/spi error
      // update status
      DEBUG_OUT("RFM95", RED, "RFM95 module not detected or SPI error\r\n");
    }

    // configure rfm95
    setOpmode(OPMODE_SLEEP);
    setOpmode(OPMODE_SLEEP | OPMODE_LONG_RANGE_MODE);

    setOpmode(OPMODE_STDBY | OPMODE_LONG_RANGE_MODE);

    // use PA_BOOST pin for higher output power
    // 1 000 1111 = 0x8F
    // use PA_BOOST pin (0b1)
    // MaxPower unused when PA_BOOST is used (0b000)
    // OutputPower = 15 (max power 17dBm) (0b1111)
    spi_.writeReg(REG_PA_CONFIG, 0x83);

    // 1001 001 0 = 0x92
    // 500kHz bandwidth (0b1001)
    // coding rate 4/5 (0b001)
    // explicit header mode (0b0)
    constexpr uint8_t modemConfig1 = 0x92;
    spi_.writeReg(REG_MODEM_CONFIG_1, modemConfig1);

    // 0111 0 1 00 = 0x74
    // spreading factor SF7 (0b0111)
    // TX continuous normal packet mode (0b0)
    // enable CRC (0b1)
    // default receiver timeout (0b00)
    constexpr uint8_t modemConfig2 = 0x74;
    spi_.writeReg(REG_MODEM_CONFIG_2, modemConfig2);

    // 0000 0 0 00 = 0x00
    // low data rate optimization off (0b0)
    // AGC auto on disabled (0b0)
    constexpr uint8_t modemConfig3 = 0x00;
    spi_.writeReg(REG_MODEM_CONFIG_3, modemConfig3);

    // set freq to 915MHz
    // Frf = (915 MHz * 2^19) / 32 MHz = 14991360 = 0xE4C000
    spi_.writeReg(REG_FRF_MSB, 0xE4);
    spi_.writeReg(REG_FRF_MID, 0xC0);
    spi_.writeReg(REG_FRF_LSB, 0x00);
  }

  void setMode(LoraMode mode) override {
    switch (mode) {
      case LoraMode::SLEEP:
        setOpmode(OPMODE_SLEEP | OPMODE_LONG_RANGE_MODE);
        break;
      case LoraMode::STANDBY:
        setOpmode(OPMODE_STDBY | OPMODE_LONG_RANGE_MODE);
        break;
      case LoraMode::TX:
        setOpmode(OPMODE_TX | OPMODE_LONG_RANGE_MODE);
        break;
      case LoraMode::RX_CONTINUOUS:
        setOpmode(OPMODE_RXCONTINUOUS | OPMODE_LONG_RANGE_MODE);
        break;
    }
  }

  bool isTransmitting() override {
    const uint8_t currentMode = spi_.readReg(REG_OP_MODE) & 0x07;
    return (currentMode == OPMODE_TX);
  }

  bool send(std::span<const uint8_t> data) override {
    if (isTransmitting()) {
      DEBUG_OUT("RFM95", RED, "Radio is currently transmitting, dropping packet of size ",
                std::to_string(data.size()), "\r\n");
      return false;
    }

    // can only write to FIFO in STDBY mode
    setMode(LoraMode::STANDBY);

    uint8_t txBaseAddr = spi_.readReg(REG_FIFO_TX_BASE_ADDR);
    spi_.writeReg(REG_FIFO_ADDR_PTR, txBaseAddr);

    spi_.writeReg(REG_PAYLOAD_LENGTH, static_cast<uint8_t>(data.size()));
    spi_.burstWrite(REG_FIFO, data);

    // clear TxDone flag
    spi_.writeReg(REG_IRQ_FLAGS, 0x08);

    setMode(LoraMode::TX);
    return true;
  }

  RxPacket receive() override {
    RxPacket p{};

    if (!packetWaiting()) {
      return p;  // no packet waiting, return empty RxPacket
    }

    const uint8_t length = spi_.readReg(REG_RX_NB_BYTES);
    const uint8_t rxAddr = spi_.readReg(REG_FIFO_RX_CURRENT_ADDR);
    spi_.writeReg(REG_FIFO_ADDR_PTR, rxAddr);
    spi_.burstRead(REG_FIFO, std::span<uint8_t>(rxBuffer_.data(), length));

    p.data = std::span<const uint8_t>(rxBuffer_.data(), length);

    auto rssi = spi_.readReg(REG_PKT_RSSI_VALUE);
    // to calculate RSSI, add -157 (from datasheet pg 107)
    p.rssi = static_cast<int16_t>(-157 + rssi);

    auto snr = spi_.readReg(REG_PKT_SNR_VALUE);
    // to calculate SNR, divide by 4 (from datasheet pg 107)
    p.snr = static_cast<float>(snr) / 4.0F;

    // clear IRQ flags (RxDone, PayloadCrcError)
    spi_.writeReg(REG_IRQ_FLAGS, 0xFF);

    return p;
  }

  bool packetWaiting() override {
    uint8_t irqFlags = spi_.readReg(REG_IRQ_FLAGS);
    // check RX_DONE bit (bit 6)
    return (irqFlags & 0x40) != 0;
  }

  void printVersion() {
    uint8_t version = spi_.readReg(REG_VERSION);
    DEBUG_OUT("RFM95", GREEN, "Version: ", std::to_string(version), "\r\n");
  }

 private:
  void setupSpi(const BoardType boardType) {
    spi_.init();
    if (boardType == BoardType::BASE_STATION) {
      spi_.configureCs(GPIOB, SPI2_CS_Pin);
    } else {
      spi_.configureCs(GPIOA, SPI2_CS_Pin);
    }
  }

  void setOpmode(const uint8_t opmode) { spi_.writeReg(REG_OP_MODE, opmode); }

  spi::Spi spi_{};

  // buffer for storing received packets
  std::array<uint8_t, Lora::RADIO_FIFO_SIZE> rxBuffer_{};
};

}  // namespace lora::rfm95