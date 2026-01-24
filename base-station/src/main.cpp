#include <FreeRTOS.h>

#include "app/app.hpp"
#include "context.hpp"
#include "task.hpp"

extern "C" void BspInit(void);
// get STM HAL peripheral handlers
// extern SPI_HandleTypeDef hspi2;

int main() {
  // init HAL and BSP
  // HAL_Init();
  BspInit();

  // instantiate task manager
  // static tasks::TaskManager taskMan;

  // instantiate drivers STATICALLY - taskman, lora, usb
  // static tasks::TaskManager taskMan;
  // pass in HAL dependencies
  // static StmLora loraDriver(&hspi2);
  // static StmUsb usbDriver;

  // create and populate context
  resources::Context context;
  // context.taskManager = &taskMan;
  // context.lora = &loraDriver;
  // context.usb = &usbDriver;
  // context.can = nullptr;
  // context.sd = nullptr;
  // context.rtc = nullptr;

  // init base-station app: should add tasks, configure app settings
  // pass in context, only store specific resources needed in app private members, dont store the
  // entire registry
  // can also setup tasks in ctor of app, tasks can be initialized in private variable section

  // start all tasks
  // taskMan.startAllTasks();
  // vTaskStartScheduler() should be the last thing called before while(true)
  vTaskStartScheduler();

  while (true) {
  }

  return 0;
}