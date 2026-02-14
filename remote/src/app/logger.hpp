#pragma once

#include "drivers/can/can.hpp"
#include "drivers/rtc/rtc.hpp"
#include "drivers/sd/sd.hpp"
#include "tasks/job.hpp"
#include "utils/utils.hpp"

namespace logger {

#pragma pack(push, 1)
struct LogFrame {
  can::CanFrame canFrame;
  // TODO: GPS
};

struct LogHeader {
  std::array<uint8_t, 9> version;  // NFR26000\n -> NFR26 v0.0.0
  rtc::RtcDate startDate;
  rtc::RtcTime startTime;
};
#pragma pack(pop)

// TODO: detect that its a new day and make a new directory if so
//      make new file with incremented name in format log00000.nfr, log00001.nfr, etc.
// need to store this in nvm somehow
// TODO: get date/time to write to RTC to reset it from laptop using preprocessor's compile time +
// date
class Logger {
 public:
  Logger(sd::SdCard& sdCard, rtc::Rtc& rtc, can::CanBus& canBus)
      : sdCard_(sdCard), rtc_(rtc), canBus_(canBus) {}
  ~Logger() = default;

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  void setup() {
    // give ptr to active RTC to static function for providing FATFS time metadata
    activeRtc_ = &rtc_;

    // setup rtc
    rtc_.init();
    rtc::RtcDate d;
    d.month = 2;
    d.day = 13;
    d.year = 26;
    d.weekday = rtc::RtcWeekday::FRIDAY;
    rtc_.setDate(d, 0xAAA0);

    rtc::RtcTime t;
    t.hours = 2;
    t.minutes = 30;
    t.seconds = 0;
    t.subseconds = 0;
    rtc_.setTime(t, 0xAAA0);

    // setup sd card
    sdCard_.init();
    sdCard_.registerTimeProvider(provideFatTime);
    sdCard_.openRollingLogFile(rtc_.getDate().toString());

    // setup can bus
    canBus_.init();

    // log header
    LogHeader header;
    header.version = version_;
    header.startDate = rtc_.getDate();
    header.startTime = rtc_.getTime();
    sdCard_.write(
        std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&header), sizeof(LogHeader)));
  }

  void simpleLog() {
    const rtc::RtcTime time = rtc_.getTime();
    const rtc::RtcDate date = rtc_.getDate();

    const std::string line =
        "Current time: " + time.toString() + ", Current date: " + date.toString() + "\r\n";

    sdCard_.write(
        std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(line.data()), line.size()));
  }

  void logCanFrame(const LogFrame& logFrame) {
    DEBUG_OUT("Logger", GREEN, "Logging CAN frame of size ", std::to_string(sizeof(LogFrame)),
              " with ID ", std::to_string(logFrame.canFrame.id), " at time ",
              std::to_string(logFrame.canFrame.timestamp), "\r\n");

    sdCard_.write(
        std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&logFrame), sizeof(LogFrame)));
  }

  void canLog() {
    can::CanFrame frame;
    while (canBus_.receive(frame, 0)) {
      LogFrame logFrame;
      logFrame.canFrame = frame;

      logCanFrame(logFrame);
    }
  }

 private:
  static uint32_t provideFatTime() {
    const rtc::RtcTime time = activeRtc_->getTime();
    const rtc::RtcDate date = activeRtc_->getDate();

    return sd::SdCard::packFatTime(date.year, date.month, date.day, time.hours, time.minutes,
                                   time.seconds);
  }

  sd::SdCard& sdCard_;
  rtc::Rtc& rtc_;
  can::CanBus& canBus_;

  const std::array<uint8_t, 9> version_ = {'N', 'F', 'R', '2', '6', '0', '0', '0', '\n'};

  inline static rtc::Rtc* activeRtc_ = nullptr;
};

class LoggerJob : public tasks::IJob {
 public:
  LoggerJob(Logger& logger) : logger_(logger) {}
  ~LoggerJob() override = default;

  // delete copy and move
  LoggerJob(const LoggerJob&) = delete;
  LoggerJob& operator=(const LoggerJob&) = delete;
  LoggerJob(LoggerJob&&) = delete;
  LoggerJob& operator=(LoggerJob&&) = delete;

  void init() override { logger_.setup(); }

  void run() override { logger_.canLog(); }

 private:
  Logger& logger_;
};

}  // namespace logger