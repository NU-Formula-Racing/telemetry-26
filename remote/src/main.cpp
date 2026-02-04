#include "main.h"

#include <FreeRTOS.h>

#include <cstring>

#include "FATFS/App/fatfs.h"
#include "USB_DEVICE/App/usbd_cdc_if.h"
#include "app/app.hpp"
#include "resources/context.hpp"
#include "tasks/task.hpp"
#include "usb_device.h"
#include "utils/utils.hpp"

// TODO: //
// do all the following in tasks:
// read from CAN
// write to CAN
// write CAN driver
// write to SD
// read from SD, less of a priority
// write SD driver
// read RTC
// write RTC driver
// write app to log

// not needed for EI MVP:
// send lora
// receive lora
// write lora driver
// clean up long ahh includes
// change cmake to lint regardless of platform

extern "C" void BspInit(void);
// get STM HAL peripheral handlers
// extern SPI_HandleTypeDef hspi2;

class PrintJob : public tasks::IJob {
 public:
  PrintJob() = default;
  ~PrintJob() override = default;

  // delete copy and move
  PrintJob(const PrintJob&) = delete;
  PrintJob& operator=(const PrintJob&) = delete;
  PrintJob(PrintJob&&) = delete;
  PrintJob& operator=(PrintJob&&) = delete;

  void init() override { DEBUG_OUT("PrintJob", GREEN, "PrintJob initialized\r\n"); }

  void run() override {
    DEBUG_OUT("PrintJob", GREEN, "printJob\r\n");
    HAL_Delay(2);
    DEBUG_OUT("PrintJob", GREEN, "printJob 2\r\n");
  }
};

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
    // DEBUG_OUT("BlinkJob", GREEN, "BlinkJob initialized\r\n");
  }

  void run() override {
    DEBUG_OUT("blinkJob", MAGENTA, "blinkJob\r\n");
    HAL_Delay(2);
    DEBUG_OUT("blinkJob", MAGENTA, "blinkJob 2\r\n");
    HAL_GPIO_TogglePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin);
    // PRINT("BlinkJob: LED toggled\r\n");
  }
};

// NEXT TODO: change this task to periodically write to the same file in the SD card
// rn it does nothing, seems to also break the freertos setup
class SdWriteJob : public tasks::IJob {
 public:
  SdWriteJob() = default;
  ~SdWriteJob() override = default;

  // delete copy and move
  SdWriteJob(const SdWriteJob&) = delete;
  SdWriteJob& operator=(const SdWriteJob&) = delete;
  SdWriteJob(SdWriteJob&&) = delete;
  SdWriteJob& operator=(SdWriteJob&&) = delete;

  void init() override {
    res_ = f_mount(&sdFatFs_, SDPath, 1);
    if (res_ != FR_OK) {
      while (true) {
        ERROR("SdWriteJob", "Mount failed, code: ", std::to_string(res_), "\r\n");
        HAL_Delay(5);
      }
    }

    res_ = f_open(&file_, "testFile.txt", FA_OPEN_ALWAYS | FA_WRITE);
    if (res_ != FR_OK) {
      while (true) {
        ERROR("SdWriteJob", "Open failed, code: ", std::to_string(res_), "\r\n");
        HAL_Delay(5);
      }
    }

    DEBUG_OUT("SdWriteJob", CYAN, "SD Card mounted and file opened\r\n");
  }

  void run() override {
    const char* line = "SD write line small\r\n";

    res_ = f_lseek(&file_, f_size(&file_));
    if (res_ != FR_OK) {
      ERROR("SdWriteJob", "Seek failed, code: ", std::to_string(res_), "\r\n");
      return;
    }

    res_ = f_write(&file_, line, static_cast<UINT>(std::strlen(line)), &bytesWritten_);
    if (res_ != FR_OK) {
      ERROR("SdWriteJob", "Write failed, code: ", std::to_string(res_), "\r\n");
      return;
    }

    DEBUG_OUT("SdWriteJob", CYAN, "tried to write ", std::to_string(bytesWritten_),
              " bytes to SD\r\n");
    f_sync(&file_);
  }

 private:
  FATFS sdFatFs_{};
  FIL file_{};
  FRESULT res_{FR_OK};
  UINT bytesWritten_{0};
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

  static PrintJob printJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> printTask(
      tasks::TaskConfig{"PrintTask", tasks::TaskPriority::LOW, 1500, printJob});

  static SdWriteJob sdWriteJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> sdWriteTask(
      tasks::TaskConfig{"SdWriteTask", tasks::TaskPriority::LOW, 3005, sdWriteJob});

  // start all tasks
  taskMan.addTask(std::move(blinkTask));
  taskMan.addTask(std::move(printTask));
  taskMan.addTask(std::move(sdWriteTask));
  taskMan.startAllTasks();
  vTaskStartScheduler();

  while (true) {
  }
}