#include "main.h"

#include <FreeRTOS.h>

#include <cstring>

#include "app/app.hpp"
#include "drivers/rtc/rtc.hpp"
#include "drivers/rtc/rtc_stm32.hpp"
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
// phase out STM main.c and init code in drivers, comment out main.c in CMakeLists

extern "C" void BspInit(void);
// get STM HAL peripheral handlers
// extern SPI_HandleTypeDef hspi2;
// extern "C" RTC_HandleTypeDef hrtc;

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
    sdCard_.write(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&num), sizeof(num)));
    num++;

    DEBUG_OUT("SdWriteJob", CYAN, "Wrote a line to SD file ", filename_, "\r\n");
  }

 private:
  sd::SdCard& sdCard_;
  std::string dirname_;
  std::string filename_;
  uint64_t num = 0;
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

class RtcReadJob : public tasks::IJob {
 public:
  RtcReadJob(rtc::Rtc& rtc) : rtc_(rtc) {}
  ~RtcReadJob() override = default;

  // delete copy and move
  RtcReadJob(const RtcReadJob&) = delete;
  RtcReadJob& operator=(const RtcReadJob&) = delete;
  RtcReadJob(RtcReadJob&&) = delete;
  RtcReadJob& operator=(RtcReadJob&&) = delete;

  void init() override {
    // HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x2345);  // writing to backup register, can check
    // this to verify RTC is setup in an init to avoid re writing time on every reset
    rtc_.init();
    rtc::RtcDate d{rtc::RtcWeekday::SATURDAY, 2, 7, 26};
    // need to do the shadow register thing i think
    rtc_.setDate(d, false);
    rtc::RtcTime t{11, 40, 0, 0};
    rtc_.setTime(t, false);
  }

  void run() override {
    //  HAL_RTC_GetTime(&hrtc, &sTime_, RTC_FORMAT_BIN);
    //  HAL_RTC_GetDate(&hrtc, &sDate_, RTC_FORMAT_BIN);

    //  const auto timeStr = "RTC Time: " + std::to_string(sTime_.Hours) + ":" +
    //                       std::to_string(sTime_.Minutes) + ":" + std::to_string(sTime_.Seconds) +
    //                       ":" + std::to_string(sTime_.SubSeconds);
    //  const auto dateStr = "RTC Date: " + std::to_string(sDate_.Month) + "/" +
    //                       std::to_string(sDate_.Date) + "/" + std::to_string(sDate_.Year);

    const rtc::RtcTime time = rtc_.getTime();
    const rtc::RtcDate date = rtc_.getDate();

    DEBUG_OUT("RtcReadJob", BLUE, time.toString().c_str(), "\r\n");
    HAL_Delay(2);
    DEBUG_OUT("RtcReadJob", BLUE, date.toString().c_str(), "\r\n");
  }

 private:
  rtc::Rtc& rtc_;
  // RTC_TimeTypeDef sTime_;
  // RTC_DateTypeDef sDate_;
};

int main() {
  BspInit();

  VERBOSITY(Verbosity::VERBOSE);

  // instantiate task manager
  static tasks::TaskManager taskMan;

  // instantiate drivers & interfaces STATICALLY
  static sd::Stm32SdDriver sdDriver;
  static sd::SdCard sd(sdDriver);
  // static rtc::Stm32RtcDriver rtcDriver(hrtc);
  static rtc::Stm32RtcDriver rtcDriver;
  static rtc::Rtc rtc(rtcDriver);
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
  ctx.rtc = &rtc;

  // init remote app: should add tasks, configure app settings
  // pass in context, only store specific resources needed in app private members, dont store the
  // entire registry
  // can also setup tasks in ctor of app, tasks can be initialized in private variable section

  static BlinkJob blinkJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> blinkTask(
      tasks::TaskConfig{"BlinkTask", tasks::TaskPriority::STANDARD, 1000, blinkJob});

  static PrintJob printJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> printTask(
      tasks::TaskConfig{"PrintTask", tasks::TaskPriority::LOW, 2500, printJob});

  static SdWriteJob sdWriteJob(*ctx.sd);
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> sdWriteTask(
      tasks::TaskConfig{"SdWriteTask", tasks::TaskPriority::LOW, 1000, sdWriteJob});

  static SdPeriodicSyncJob sdSyncJob(*ctx.sd);
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> sdPeriodicSyncTask(
      tasks::TaskConfig{"SdSyncTask", tasks::TaskPriority::STANDARD, 5000, sdSyncJob});

  static RtcReadJob rtcReadJob(*ctx.rtc);
  // static RtcReadJob rtcReadJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::MEDIUM> rtcReadTask(
      tasks::TaskConfig{"RtcReadTask", tasks::TaskPriority::STANDARD, 300, rtcReadJob});

  // start all tasks
  taskMan.addTask(std::move(blinkTask));
  taskMan.addTask(std::move(printTask));
  taskMan.addTask(std::move(sdWriteTask));
  taskMan.addTask(std::move(sdPeriodicSyncTask));
  taskMan.addTask(std::move(rtcReadTask));
  taskMan.startAllTasks();
  vTaskStartScheduler();

  while (true) {
  }
}