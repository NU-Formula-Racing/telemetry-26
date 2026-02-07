#include "main.h"

#include <FreeRTOS.h>

#include <cstring>

#include "FATFS/App/fatfs.h"
#include "USB_DEVICE/App/usbd_cdc_if.h"
#include "app/app.hpp"
#include "drivers/sd/sd.hpp"
#include "drivers/sd/sd_stm32.hpp"
#include "resources/context.hpp"
#include "tasks/task.hpp"
#include "usb_device.h"
#include "utils/utils.hpp"

// TODO: //
// read from CAN in ISR, task
// write to CAN in task
// write CAN driver, make PR
// read from SD, less of a priority
// read RTC
// write RTC driver
// write app to log

// not needed for EI MVP:
// send lora
// receive lora
// write lora driver
// GPS
// read from sd
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
  }

  void run() override {
    DEBUG_OUT("blinkJob", MAGENTA, "blinkJob\r\n");
    HAL_Delay(2);
    DEBUG_OUT("blinkJob", MAGENTA, "blinkJob 2\r\n");
    HAL_GPIO_TogglePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin);
  }
};

class SdWriteJob : public tasks::IJob {
 public:
  SdWriteJob(sd::SdCard& sdCard) : sdCard_(sdCard) {}
  ~SdWriteJob() override = default;

  // delete copy and move
  SdWriteJob(const SdWriteJob&) = delete;
  SdWriteJob& operator=(const SdWriteJob&) = delete;
  SdWriteJob(SdWriteJob&&) = delete;
  SdWriteJob& operator=(SdWriteJob&&) = delete;

  void init() override {
    uint8_t mode = sd::SdFileMode::WRITE | sd::SdFileMode::OPEN_ALWAYS;
    dirname_ = "testdir";
    filename_ = "testdir/test.nfr";
    sdCard_.init();
    sdCard_.mkdir(dirname_);
    sdCard_.openFile(filename_, mode);
  }

  void run() override {
    const std::string line = "hi nfr\r\n";

    sdCard_.write(
        std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(line.data()), line.size()));

    DEBUG_OUT("SdWriteJob", CYAN, "Wrote a line to SD file ", filename_, "\r\n");
  }

 private:
  sd::SdCard& sdCard_;
  std::string dirname_;
  std::string filename_;
};

class SdPeriodicSyncJob : public tasks::IJob {
 public:
  SdPeriodicSyncJob(sd::SdCard& sdCard) : sdCard_(sdCard) {}
  ~SdPeriodicSyncJob() override = default;

  // delete copy and move
  SdPeriodicSyncJob(const SdPeriodicSyncJob&) = delete;
  SdPeriodicSyncJob& operator=(const SdPeriodicSyncJob&) = delete;
  SdPeriodicSyncJob(SdPeriodicSyncJob&&) = delete;
  SdPeriodicSyncJob& operator=(SdPeriodicSyncJob&&) = delete;

  void init() override {}

  void run() override {
    sdCard_.periodicSync();
    DEBUG_OUT("SdPeriodicSyncJob", CYAN, "Flushed SD card buffers\r\n");
  }

 private:
  sd::SdCard& sdCard_;
};

int main() {
  BspInit();

  VERBOSITY(Verbosity::VERBOSE);

  // instantiate task manager
  static tasks::TaskManager taskMan;

  // instantiate drivers & interfaces STATICALLY
  static sd::Stm32SdDriver sdDriver;
  static sd::SdCard sd(sdDriver);
  // pass in HAL dependencies
  // static StmLora loraDriver(&hspi2);
  // static StmUsb usbDriver;

  // create and populate context
  static resources::Context ctx;
  ctx.taskManager = &taskMan;
  // ctx.lora = &lora;
  // ctx.usb = nullptr;
  // ctx.can = &can;
  ctx.sd = &sd;
  // ctx.rtc = &rtc;

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

  static SdWriteJob sdWriteJob(*ctx.sd);
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> sdWriteTask(
      tasks::TaskConfig{"SdWriteTask", tasks::TaskPriority::LOW, 10, sdWriteJob});

  static SdPeriodicSyncJob sdSyncJob(*ctx.sd);
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> sdPeriodicSyncTask(
      tasks::TaskConfig{"SdSyncTask", tasks::TaskPriority::STANDARD, 500, sdSyncJob});

  // start all tasks
  taskMan.addTask(std::move(blinkTask));
  taskMan.addTask(std::move(printTask));
  taskMan.addTask(std::move(sdWriteTask));
  taskMan.addTask(std::move(sdPeriodicSyncTask));
  taskMan.startAllTasks();
  vTaskStartScheduler();

  while (true) {
  }
}