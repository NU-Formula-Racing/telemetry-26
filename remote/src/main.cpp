#include "main.h"

#include <FreeRTOS.h>

#include <cstring>

#include "app/logger.hpp"
#include "app/remote.hpp"
#include "app/wireless.hpp"
#include "drivers/can/can_stm32.hpp"
#include "drivers/lora/rfm95.hpp"
#include "drivers/rtc/rtc_stm32.hpp"
#include "drivers/sd/sd_stm32.hpp"
#include "drivers/servo/servo_stm32.hpp"
#include "tasks/task.hpp"
#include "utils/utils.hpp"

// TODO //
// protocol
// usb class
// CAN status msg

// someday:
// use DIOs + interrupts
// change cmake to lint regardless of platform
// phase out STM main.c and init code in drivers, comment out main.c in CMakeLists
// std::source_location instead of __FILE__ and __LINE__

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
  static lora::Lora lora(rfm95);

  static servo::Stm32ServoDriver servoDriver;
  static aero::ActiveAeroController activeAero(servoDriver);

  // instantiate apps
  static logger::Logger logger(sd, rtc);
  static wireless::Wireless wireless(lora);
  // static remote::status status;
  static remote::Remote remote(logger, wireless, can, activeAero);

  // setup tasks
  static /*remote::*/ BlinkJob blinkJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> blinkTask(
      tasks::TaskConfig{"BlinkTask", tasks::TaskPriority::STANDARD, 1000, blinkJob});

  static remote::ProcessCanJob processCanJob(remote);
  static tasks::FreeRtosTask<tasks::TaskStackSize::XLARGE> processCanTask(
      tasks::TaskConfig{"ProcessCanTask", tasks::TaskPriority::HIGH, 10, processCanJob});

  static logger::SdWriteJob sdWriteJob(sd);
  static tasks::FreeRtosTask<tasks::TaskStackSize::XLARGE> sdWriteTask(
      tasks::TaskConfig{"SdWriteTask", tasks::TaskPriority::LOW, 10, sdWriteJob});

  static wireless::LoraWriteJob loraWriteJob(wireless);
  static tasks::FreeRtosTask<tasks::TaskStackSize::XLARGE> loraWriteTask(
      tasks::TaskConfig{"LoraWriteTask", tasks::TaskPriority::LOW, 200, loraWriteJob});

  static RtcPrintJob rtcPrintJob(rtc);
  static tasks::FreeRtosTask<tasks::TaskStackSize::LARGE> rtcPrintTask(
      tasks::TaskConfig{"RtcPrintTask", tasks::TaskPriority::STANDARD, 2000, rtcPrintJob});

  static remote::OdometerCanTxJob odometerCanTxJob(remote);
  static tasks::FreeRtosTask<tasks::TaskStackSize::LARGE> odometerCanTxTask(
      tasks::TaskConfig{"OdometerCanTxTask", tasks::TaskPriority::STANDARD, 100, odometerCanTxJob});

  static remote::RtcCanTxJob rtcCanTxJob(remote);
  static tasks::FreeRtosTask<tasks::TaskStackSize::MEDIUM> rtcCanTxTask(
      tasks::TaskConfig{"RtcCanTxTask", tasks::TaskPriority::LOW, 1000, rtcCanTxJob});

  static remote::StatusCanTxJob statusCanTxJob(remote);
  static tasks::FreeRtosTask<tasks::TaskStackSize::LARGE> statusCanTxTask(
      tasks::TaskConfig{"StatusCanTxTask", tasks::TaskPriority::LOW, 1002, statusCanTxJob});

  static remote::ActiveAeroCtrlJob activeAeroCtrlJob(remote);
  static tasks::FreeRtosTask<tasks::TaskStackSize::LARGE> activeAeroCtrlTask(
      tasks::TaskConfig{"ActiveAeroCtrlTask", tasks::TaskPriority::LOW, 1003, activeAeroCtrlJob});

  // start all tasks
  taskMan.addTask(std::move(blinkTask));
  taskMan.addTask(std::move(processCanTask));
  taskMan.addTask(std::move(sdWriteTask));
  taskMan.addTask(std::move(loraWriteTask));
  taskMan.addTask(std::move(rtcPrintTask));
  taskMan.addTask(std::move(odometerCanTxTask));
  taskMan.addTask(std::move(rtcCanTxTask));
  taskMan.addTask(std::move(statusCanTxTask));
  taskMan.addTask(std::move(activeAeroCtrlTask));
  taskMan.startAllTasks();

  vTaskStartScheduler();

  while (true) {
  }
}