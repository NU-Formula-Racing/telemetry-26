#include "main.h"

#include <FreeRTOS.h>

#include <cstring>

#include "app/logger.hpp"
#include "app/remote.hpp"
#include "app/status.hpp"
#include "app/wireless.hpp"
#include "drivers/can/can_stm32.hpp"
#include "drivers/lora/rfm95.hpp"
#include "drivers/lora/spi.hpp"
#include "drivers/rtc/rtc_stm32.hpp"
#include "drivers/sd/sd_stm32.hpp"
#include "tasks/task.hpp"
#include "utils/utils.hpp"

// TODO: //
// CAN status msg
// dont break when theres no SD card lol
// delete context

// not needed for EI MVP:
// send lora
// receive lora
// write lora driver
// read from sd
// write CAN driver, make PR
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

  void init() override { HAL_GPIO_WritePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin, GPIO_PIN_RESET); }

  void run() override { HAL_GPIO_TogglePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin); }
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

  static lora::rfm95::Rfm95 rfm95;

  // instantiate apps
  static logger::Logger logger(sd, rtc);
  static wireless::Wireless wireless(rfm95);
  static remote::Remote remote(logger, wireless, can);

  // setup tasks
  static /*remote::*/ BlinkJob blinkJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> blinkTask(
      tasks::TaskConfig{"BlinkTask", tasks::TaskPriority::STANDARD, 1000, blinkJob});

  static remote::ProcessCanJob processCanJob(remote);
  static tasks::FreeRtosTask<tasks::TaskStackSize::XLARGE> processCanTask(
      tasks::TaskConfig{"ProcessCanTask", tasks::TaskPriority::HIGH, 10, processCanJob});

  static logger::SdWriteJob sdWriteJob(sd);
  static tasks::FreeRtosTask<tasks::TaskStackSize::XLARGE> sdWriteTask(
      tasks::TaskConfig{"SdWriteTask", tasks::TaskPriority::LOW, 500, sdWriteJob});

  // static rfm95::LoraJob loraJob(rfm95);
  // static tasks::FreeRtosTask<tasks::TaskStackSize::XLARGE> loraWriteTask(
  //     tasks::TaskConfig{"LoraWriteTask", tasks::TaskPriority::LOW, 100, loraWriteJob});

  static RtcPrintJob rtcPrintJob(rtc);
  static tasks::FreeRtosTask<tasks::TaskStackSize::MEDIUM> rtcPrintTask(
      tasks::TaskConfig{"RtcPrintTask", tasks::TaskPriority::STANDARD, 2000, rtcPrintJob});

  static rfm95::LoraJob loraJob(rfm95);
  static tasks::FreeRtosTask<tasks::TaskStackSize::LARGE> loraTask(
      tasks::TaskConfig{"LoraTask", tasks::TaskPriority::STANDARD, 1000, loraJob});

  // start all tasks
  taskMan.addTask(std::move(blinkTask));
  taskMan.addTask(std::move(processCanTask));
  taskMan.addTask(std::move(sdWriteTask));
  // taskMan.addTask(std::move(loraWriteTask));
  taskMan.addTask(std::move(rtcPrintTask));
  taskMan.addTask(std::move(loraTask));
  taskMan.startAllTasks();

  vTaskStartScheduler();

  while (true) {
  }
}