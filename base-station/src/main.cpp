#include "main.h"

#include <FreeRTOS.h>

#include <cstring>

#include "app/base_station.hpp"
#include "resources/context.hpp"
#include "stm32f4xx_hal_spi.h"
#include "tasks/task.hpp"
#include "utils/utils.hpp"

extern "C" void BspInit(void);

class BlinkJob : public tasks::IJob {
 public:
  BlinkJob() = default;
  ~BlinkJob() override = default;

  // delete copy and move
  BlinkJob(const BlinkJob&) = delete;
  BlinkJob& operator=(const BlinkJob&) = delete;
  BlinkJob(BlinkJob&&) = delete;
  BlinkJob& operator=(BlinkJob&&) = delete;

  void init() override {
    HAL_GPIO_WritePin(LORA_STATUS_GPIO_Port, LORA_STATUS_Pin, GPIO_PIN_RESET);  // turn led off
  }

  void run() override {
    DEBUG_OUT("BlinkJob", MAGENTA, "blinking led\r\n");
    HAL_GPIO_TogglePin(LORA_STATUS_GPIO_Port, LORA_STATUS_Pin);
  }
};

class LoraRxJob : public tasks::IJob {
 public:
  LoraRxJob(SPI_HandleTypeDef& hspi) : hspi(hspi) {};
  ~LoraRxJob() override = default;

  // delete copy and move
  LoraRxJob(const LoraRxJob&) = delete;
  LoraRxJob& operator=(const LoraRxJob&) = delete;
  LoraRxJob(LoraRxJob&&) = delete;
  LoraRxJob& operator=(LoraRxJob&&) = delete;

  void init() override {}

  void run() override {
    // TODO: receive a simple message over lora
  }

 private:
  SPI_HandleTypeDef& hspi;

  uint8_t readReg(uint8_t reg) {
    std::array<uint8_t, 2> tx = {static_cast<uint8_t>(reg & 0x7F), 0x00};
    std::array<uint8_t, 2> rx = {0};

    HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi, tx.data(), rx.data(), 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_SET);

    return rx.at(1);
  }

  void writeReg(uint8_t reg, uint8_t val) {
    std::array<uint8_t, 2> tx = {static_cast<uint8_t>(reg | 0x80), val};

    HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi, tx.data(), 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, SPI2_CS_Pin, GPIO_PIN_SET);
  }
};

int main() {
  BspInit();

  VERBOSITY(Verbosity::VERBOSE);

  // instantiate task manager
  static tasks::TaskManager taskMan;

  // instantiate drivers & interfaces STATICALLY
  extern SPI_HandleTypeDef hspi2;

  // create and populate context
  static resources::Context ctx;
  ctx.taskManager = &taskMan;
  // ctx.lora = &lora;
  // ctx.usb = nullptr;
  // ctx.can = &can;
  // ctx.sd = &sd;
  // ctx.rtc = &rtc;

  // instantiate apps
  // static base::BaseStation baseStation(lora, usb);

  // setup tasks
  static BlinkJob blinkJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> blinkTask(
      tasks::TaskConfig{"BlinkTask", tasks::TaskPriority::STANDARD, 1000, blinkJob});

  static LoraRxJob loraRxJob(hspi2);
  static tasks::FreeRtosTask<tasks::TaskStackSize::MEDIUM> loraRxTask(
      tasks::TaskConfig{"LoraRxTask", tasks::TaskPriority::HIGH, 500, loraRxJob});

  // start all tasks
  taskMan.addTask(std::move(blinkTask));
  taskMan.addTask(std::move(loraRxTask));

  taskMan.startAllTasks();
  vTaskStartScheduler();

  while (true) {
  }
}