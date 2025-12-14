#include <FreeRTOS.h>

#include "app/app.hpp"
#include "task.hpp"

extern "C" void BspInit(void);
// get STM HAL peripheral handlers
// extern SPI_HandleTypeDef hspi2;

int main() {
  BspInit();

  // instantiate task manager
  // static tasks::TaskManager taskMan;

  // instantiate drivers - lora, usb
  // pass in HAL dependencies
  // static StmLora loraDriver(&hspi2);
  // static StmUsb usbDriver;

  // create registry
  // TelemetryRegistry registry;
  // registry.taskManager = &taskMan;
  // registry.lora = &loraDriver;
  // registry.usb = &usbDriver;
  // registry.can = nullptr;
  // registry.sd = nullptr;

  // vTaskStartScheduler() should be the last thing called before while(true)
  // vTaskStartScheduler();

  while (true) {
  }

  return 0;
}