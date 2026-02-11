#pragma once

#include "drivers/rtc/rtc.hpp"
#include "drivers/sd/sd.hpp"
// #include "tasks/tasks.hpp"
#include "tasks/job.hpp"
#include "utils/utils.hpp"

namespace logger {

#pragma pack(push, 1)
struct LogFrame {
  rtc::RtcTime time;
  uint32_t id;  // 12 bit standard ID or 29 bit extended ID
  uint8_t len;  // data length in bits
  std::array<uint8_t, 8> data;
};

struct LogHeader {
  std::array<uint8_t, 9> version;  // NFR26000\n -> NFR26 v0.0.0
  rtc::RtcDate startDate;
  rtc::RtcTime startTime;
};
#pragma pack(pop)

class Logger {
 public:
  Logger(sd::SdCard& sdCard, rtc::Rtc& rtc) : sdCard_(sdCard), rtc_(rtc) {}
  ~Logger() = default;

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  void setup() {
    uint8_t mode = sd::SdFileMode::WRITE | sd::SdFileMode::OPEN_ALWAYS;
    sdCard_.init();
    sdCard_.mkdir("rtctest");
    sdCard_.openFile("rtctest/rtc0004.nfr", mode);

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

    // log header
    LogHeader header;
    header.version = version_;
    header.startDate = rtc_.getDate();
    header.startTime = rtc_.getTime();
    sdCard_.write(
        std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&header), sizeof(LogHeader)));
  }

  void log() {
    const rtc::RtcTime time = rtc_.getTime();
    const rtc::RtcDate date = rtc_.getDate();

    const std::string line =
        "Current time: " + time.toString() + ", Current date: " + date.toString() + "\r\n";

    sdCard_.write(
        std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(line.data()), line.size()));
  }

 private:
  sd::SdCard& sdCard_;
  rtc::Rtc& rtc_;
  const std::array<uint8_t, 9> version_ = {'N', 'F', 'R', '2', '6', '0', '0', '0', '\n'};
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

  void run() override { logger_.log(); }

 private:
  Logger& logger_;
};

}  // namespace logger