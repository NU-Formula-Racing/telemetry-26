#include "main.h"

#include <FreeRTOS.h>

#include <cstdint>
#include <cstring>
#include <memory>

#include "app/heartbeat.hpp"
#include "app/logger.hpp"
#include "drivers/can/can_stm32.hpp"
#include "drivers/rtc/rtc_stm32.hpp"
#include "drivers/sd/sd_stm32.hpp"
#include "resources/context.hpp"
#include "tasks/task.hpp"
#include "utils/utils.hpp"

// TODO: //
// write CAN driver, make PR
// come up iwth a way to automatically increment log file names - writing to some flash register ?

// not needed for EI MVP:
// hardware timer instead of RTC for more accurate logging timestamps
// send lora
// receive lora
// write lora driver
// GPS
// read from sd
// clean up long ahh includes
// change cmake to lint regardless of platform
// phase out STM main.c and init code in drivers, comment out main.c in CMakeLists
// write a heartbeat class and driver for gpios/leds

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
    HAL_GPIO_WritePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin, GPIO_PIN_RESET);  // turn led off
  }

  void run() override {
    DEBUG_OUT("blinkJob", MAGENTA, "blinkJob\r\n");
    HAL_GPIO_TogglePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin);
  }
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

class RtcPrintJob : public tasks::IJob {
 public:
  RtcPrintJob(rtc::Rtc& rtc) : rtc_(rtc) {}
  ~RtcPrintJob() override = default;

  // delete copy and move
  RtcPrintJob(const RtcPrintJob&) = delete;
  RtcPrintJob& operator=(const RtcPrintJob&) = delete;
  RtcPrintJob(RtcPrintJob&&) = delete;
  RtcPrintJob& operator=(RtcPrintJob&&) = delete;

  void init() override {}

  void run() override {
    const rtc::RtcTime time = rtc_.getTime();
    const rtc::RtcDate date = rtc_.getDate();

    DEBUG_OUT("RtcPrintJob", YELLOW, "Current time: ", time.toString().c_str(),
              ", Current date: ", date.toString().c_str(), "\r\n");
  }

 private:
  rtc::Rtc& rtc_;
};

int main() {
  BspInit();

  VERBOSITY(Verbosity::VERBOSE);

  // instantiate task manager
  static tasks::TaskManager taskMan;

  // instantiate drivers & interfaces STATICALLY
  static sd::Stm32SdDriver sdDriver;
  static sd::SdCard sd(sdDriver);

  static rtc::Stm32RtcDriver rtcDriver;
  static rtc::Rtc rtc(rtcDriver);

  static can::Stm32CanDriver canDriver;
  static can::CanBus can(canDriver);

  // create and populate context
  static resources::Context ctx;
  ctx.taskManager = &taskMan;
  // ctx.lora = &lora;
  // ctx.usb = nullptr;
  ctx.can = &can;
  ctx.sd = &sd;
  ctx.rtc = &rtc;

  // instantiate apps
  static logger::Logger logger(sd, rtc, can);

  // setup tasks
  static BlinkJob blinkJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> blinkTask(
      tasks::TaskConfig{"BlinkTask", tasks::TaskPriority::STANDARD, 1000, blinkJob});

  static logger::LoggerJob loggerJob(logger);
  static tasks::FreeRtosTask<tasks::TaskStackSize::LARGE> loggerTask(
      tasks::TaskConfig{"LoggerTask", tasks::TaskPriority::STANDARD, 100, loggerJob});

  static SdPeriodicSyncJob sdSyncJob(*ctx.sd);
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> sdPeriodicSyncTask(
      tasks::TaskConfig{"SdSyncTask", tasks::TaskPriority::STANDARD, 500, sdSyncJob});

  static RtcPrintJob rtcPrintJob(*ctx.rtc);
  static tasks::FreeRtosTask<tasks::TaskStackSize::MEDIUM> rtcPrintTask(
      tasks::TaskConfig{"RtcPrintTask", tasks::TaskPriority::STANDARD, 2000, rtcPrintJob});

  // start all tasks
  taskMan.addTask(std::move(blinkTask));
  taskMan.addTask(std::move(loggerTask));
  taskMan.addTask(std::move(sdPeriodicSyncTask));
  taskMan.addTask(std::move(rtcPrintTask));

  taskMan.startAllTasks();
  vTaskStartScheduler();

  while (true) {
  }
}