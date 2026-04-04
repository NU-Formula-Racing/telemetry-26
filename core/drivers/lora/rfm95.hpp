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

  void init(LoraConfig config) override {
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
    }

    // configure rfm95
    spi_.writeReg(REG_OP_MODE, OPMODE_SLEEP);
    // vTaskDelay(pdMS_TO_TICKS(10));
    spi_.writeReg(REG_OP_MODE, OPMODE_SLEEP | OPMODE_STDBY);

    // set freq to 915MHz
    // Frf = (915 MHz * 2^19) / 32 MHz = 14991360 = 0xE4C000
    spi_.writeReg(REG_FRF_MSB, 0xE4);
    spi_.writeReg(REG_FRF_MID, 0xC0);
    spi_.writeReg(REG_FRF_LSB, 0x00);
  }

  void setOpmode(const uint8_t opmode) { spi_.writeReg(REG_OP_MODE, opmode); }

  void send(const uint8_t* data, size_t len) override {
    const uint8_t opmode = spi_.readReg(REG_OP_MODE);
    if ((opmode & 0x3) != OPMODE_TX && (opmode & 0x3) != OPMODE_FSTX) {
      // TODO: not in tx mode, update status
    }
    //
  }

  void receive(uint8_t* /*buffer*/, size_t /*len*/) override {
    // TODO
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

  spi::Spi spi_;
};

class LoraJob : public tasks::IJob {
 public:
  LoraJob(Rfm95& rfm95) : rfm95_(rfm95) {}
  ~LoraJob() override = default;

  // delete copy and move
  LoraJob(const LoraJob&) = delete;
  LoraJob& operator=(const LoraJob&) = delete;
  LoraJob(LoraJob&&) = delete;
  LoraJob& operator=(LoraJob&&) = delete;

  void init() override { rfm95_.init(LoraConfig{.boardType = BoardType::BASE_STATION}); }

  void run() override { rfm95_.printVersion(); }

 private:
  Rfm95& rfm95_;
};

}  // namespace lora::rfm95