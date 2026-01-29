#include "../bsp/Core/Inc/main.h"

#include <FreeRTOS.h>

#include <cstring>

#include "../../core/resources/context.hpp"
#include "../../core/tasks/task.hpp"
#include "../../core/utils/utils.hpp"
#include "../bsp/USB_DEVICE/App/usb_device.h"
#include "../bsp/USB_DEVICE/App/usbd_cdc_if.h"
#include "app/app.hpp"

// TODO: //
// write usb debug
// do all the following in tasks:
// read from CAN
// write to CAN
// write CAN driver
// write to SD
// read from SD, less of a priority
// write SD driver
// read RTC
// write RTC driver

// not needed for EI MVP:
// send lora
// receive lora
// write lora driver
// clean up long ahh includes
// change cmake to lint regardless of platform

extern "C" void BspInit(void);
// get STM HAL peripheral handlers
// extern SPI_HandleTypeDef hspi2;

// extremely simple test blinky for now
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
    HAL_GPIO_WritePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin, GPIO_PIN_RESET);  // turn led off
    DEBUG_OUT("BlinkJob", GREEN, "BlinkJob initialized\r\n");
  }

  void run() override {
    HAL_GPIO_TogglePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin);
    DEBUG_OUT("blinkJob", MAGENTA, "Hello World\r\n");
    PRINT("BlinkJob: LED toggled\r\n");
  }
};

int main() {
  BspInit();

  VERBOSITY(Verbosity::VERBOSE);

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

  static BlinkJob blinkJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> blinkTask(
      tasks::TaskConfig{"BlinkTask", tasks::TaskPriority::STANDARD, 1000, blinkJob});

  // start all tasks
  taskMan.addTask(std::move(blinkTask));
  taskMan.startAllTasks();
  vTaskStartScheduler();

  while (true) {
  }
}