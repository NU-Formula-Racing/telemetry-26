#include "main.h"

#include <FreeRTOS.h>

#include <cstring>

#include "app/logger.hpp"
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
// write app to log

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

  void run() override { DEBUG_OUT("PrintJob", GREEN, "printJob\r\n"); }
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
    rtc_.init();
    rtc::RtcDate d;
    d.month = 2;
    d.day = 8;
    d.year = 26;
    d.weekday = rtc::RtcWeekday::SUNDAY;
    rtc_.setDate(d, 0x0001);

    rtc::RtcTime t;
    t.hours = 16;
    t.minutes = 59;
    t.seconds = 0;
    t.subseconds = 0;
    rtc_.setTime(t, 0x0001);
  }

  void run() override {
    const rtc::RtcTime time = rtc_.getTime();
    const rtc::RtcDate date = rtc_.getDate();

    DEBUG_OUT("RtcReadJob", BLUE, time.toString().c_str(), "\r\n");
    HAL_Delay(2);
    DEBUG_OUT("RtcReadJob", BLUE, date.toString().c_str(), "\r\n");
  }

 private:
  rtc::Rtc& rtc_;
};

class RtcWriteToSdJob : public tasks::IJob {
 public:
  RtcWriteToSdJob(rtc::Rtc& rtc, sd::SdCard& sdCard) : rtc_(rtc), sdCard_(sdCard) {}
  ~RtcWriteToSdJob() override = default;

  // delete copy and move
  RtcWriteToSdJob(const RtcWriteToSdJob&) = delete;
  RtcWriteToSdJob& operator=(const RtcWriteToSdJob&) = delete;
  RtcWriteToSdJob(RtcWriteToSdJob&&) = delete;
  RtcWriteToSdJob& operator=(RtcWriteToSdJob&&) = delete;

  void init() override {
    // setup sd card
    uint8_t mode = sd::SdFileMode::WRITE | sd::SdFileMode::OPEN_ALWAYS;
    dirname_ = "rtctest";
    filename_ = "rtctest/rtc0002.nfr";
    sdCard_.init();
    sdCard_.mkdir(dirname_);
    sdCard_.openFile(filename_, mode);

    // setup rtc
    rtc_.init();
    rtc::RtcDate d;
    d.month = 2;
    d.day = 8;
    d.year = 26;
    d.weekday = rtc::RtcWeekday::SUNDAY;
    rtc_.setDate(d, 0x0001);

    rtc::RtcTime t;
    t.hours = 16;
    t.minutes = 59;
    t.seconds = 0;
    t.subseconds = 0;
    rtc_.setTime(t, 0x0001);
  }

  void run() override {
    const rtc::RtcTime time = rtc_.getTime();
    const rtc::RtcDate date = rtc_.getDate();

    const std::string line =
        "Current time: " + time.toString() + ", Current date: " + date.toString() + "\r\n";

    sdCard_.write(
        std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(line.data()), line.size()));

    // DEBUG_OUT("RtcWriteToSdJob", YELLOW, "Wrote current RTC time ", time.toString().c_str(),
    //           " and date ", date.toString().c_str(), " to SD card\r\n");
  }

 private:
  rtc::Rtc& rtc_;
  sd::SdCard& sdCard_;
  std::string dirname_;
  std::string filename_;
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

  // create and populate context
  static resources::Context ctx;
  ctx.taskManager = &taskMan;
  // ctx.lora = &lora;
  // ctx.usb = nullptr;
  // ctx.can = &can;
  ctx.sd = &sd;
  ctx.rtc = &rtc;

  // instantiate apps
  static logger::Logger logger(sd, rtc);

  // setup tasks
  static BlinkJob blinkJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> blinkTask(
      tasks::TaskConfig{"BlinkTask", tasks::TaskPriority::STANDARD, 1000, blinkJob});

  static PrintJob printJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> printTask(
      tasks::TaskConfig{"PrintTask", tasks::TaskPriority::LOW, 2500, printJob});

  static logger::LoggerJob loggerJob(logger);
  static tasks::FreeRtosTask<tasks::TaskStackSize::MEDIUM> loggerTask(
      tasks::TaskConfig{"LoggerTask", tasks::TaskPriority::STANDARD, 10, loggerJob});

  // static RtcWriteToSdJob rtcWriteToSdJob(*ctx.rtc, *ctx.sd);
  // static tasks::FreeRtosTask<tasks::TaskStackSize::MEDIUM> rtcWriteToSdTask(
  //     tasks::TaskConfig{"RtcWriteToSdTask", tasks::TaskPriority::STANDARD, 10, rtcWriteToSdJob});

  // static SdWriteJob sdWriteJob(*ctx.sd);
  // static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> sdWriteTask(
  //     tasks::TaskConfig{"SdWriteTask", tasks::TaskPriority::LOW, 1000, sdWriteJob});

  static SdPeriodicSyncJob sdSyncJob(*ctx.sd);
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> sdPeriodicSyncTask(
      tasks::TaskConfig{"SdSyncTask", tasks::TaskPriority::STANDARD, 500, sdSyncJob});

  // static RtcReadJob rtcReadJob(*ctx.rtc);
  // static tasks::FreeRtosTask<tasks::TaskStackSize::MEDIUM> rtcReadTask(
  //     tasks::TaskConfig{"RtcReadTask", tasks::TaskPriority::STANDARD, 300, rtcReadJob});

  // start all tasks
  taskMan.addTask(std::move(blinkTask));
  taskMan.addTask(std::move(printTask));
  // taskMan.addTask(std::move(rtcWriteToSdTask));
  // taskMan.addTask(std::move(sdWriteTask));
  taskMan.addTask(std::move(loggerTask));
  taskMan.addTask(std::move(sdPeriodicSyncTask));
  // taskMan.addTask(std::move(rtcReadTask));
  taskMan.startAllTasks();
  vTaskStartScheduler();

  while (true) {
  }
}