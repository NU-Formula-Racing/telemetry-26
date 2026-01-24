#include "../bsp/Core/Inc/main.h"

#include <FreeRTOS.h>

#include <cstring>

#include "../../core/resources/context.hpp"
#include "../../core/tasks/task.hpp"
#include "../bsp/USB_DEVICE/App/usb_device.h"
#include "../bsp/USB_DEVICE/App/usbd_cdc_if.h"
#include "app/app.hpp"

extern "C" void BspInit(void);
// get STM HAL peripheral handlers
// extern SPI_HandleTypeDef hspi2;

int main() {
  BspInit();
  // instantiate task manager
  static tasks::TaskManager taskMan;

  // instantiate drivers STATICALLY - lora, usb
  // pass in HAL dependencies
  // static StmLora loraDriver(&hspi2);
  // static StmUsb usbDriver;

  // create and populate context
  static resources::Context ctx;
  ctx.taskManager = &taskMan;
  // ctx.lora = &loraDriver;
  // ctx.usb = &usbDriver;
  // ctx.can = nullptr;
  // ctx.sd = nullptr;
  // ctx.rtc = nullptr;

  // init remote app: should add tasks, configure app settings
  // pass in context, only store specific resources needed in app private members, dont store the
  // entire registry
  // can also setup tasks in ctor of app, tasks can be initialized in private variable section

  // start all tasks
  taskMan.startAllTasks();
  // vTaskStartScheduler() should be the last thing called before while(true)
  vTaskStartScheduler();

  while (true) {
    HAL_GPIO_TogglePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin);
    const char* msg = "Hello World\r\n";
    CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
    HAL_Delay(1000);
  }
}