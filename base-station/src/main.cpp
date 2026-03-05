#include "main.h"

#include <FreeRTOS.h>

#include <cstring>

#include "app/base_station.hpp"
#include "resources/context.hpp"
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

int main() {
  BspInit();

  VERBOSITY(Verbosity::VERBOSE);

  // instantiate task manager
  static tasks::TaskManager taskMan;

  // instantiate drivers & interfaces STATICALLY

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
  // static BlinkJob blinkJob;
  // static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> blinkTask(
  //    tasks::TaskConfig{"BlinkTask", tasks::TaskPriority::STANDARD, 1000, blinkJob});

  // start all tasks
  // taskMan.addTask(std::move(blinkTask));

  // taskMan.startAllTasks();
  // vTaskStartScheduler();

  while (true) {
    const char* msg = "cmake\r\n";

    // 3. Transmit over USB
    // CDC_Transmit_FS takes a uint8_t pointer and the length of the data
    CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
    // DEBUG_OUT("BlinkJob", MAGENTA, "blinking led\r\n");
    HAL_GPIO_TogglePin(LORA_STATUS_GPIO_Port, LORA_STATUS_Pin);
    HAL_Delay(500);
  }
}