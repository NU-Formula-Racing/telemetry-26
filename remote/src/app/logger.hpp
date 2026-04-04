#pragma once

#include <cstdint>

#include "drivers/can/can.hpp"
#include "drivers/rtc/rtc.hpp"
#include "drivers/rtc/rtc_utils.hpp"
#include "drivers/sd/sd.hpp"
#include "tasks/job.hpp"
#include "utils/utils.hpp"

namespace logger {

#pragma pack(push, 1)
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

  void init() {
    // give ptr to active RTC to static function for providing FATFS time metadata
    activeRtc_ = &rtc_;

    // setup rtc
    rtc_.init();
    constexpr rtc::RtcDate compileDate = rtc::utils::parseCompilerDate(__DATE__);
    constexpr rtc::RtcTime compileTime = rtc::utils::parseCompilerTime(__TIME__);
    rtc_.setDate(compileDate, 0xAAAA);
    rtc_.setTime(compileTime, 0xAAAA);

    // setup sd card
    sdCard_.init();
    sdCard_.registerTimeProvider(provideFatTime);
    sdCard_.openRollingLogFile(rtc_.getDate().toString());

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

  void logCanFrame(const can::CanFrame& canFrame) {
    // DEBUG_OUT("Logger", GREEN, "Logging CAN frame of size ", std::to_string(sizeof(LogFrame)),
    //           " with ID ", std::to_string(logFrame.canFrame.id), " at time ",
    //           std::to_string(logFrame.canFrame.timestamp), "\r\n");

    sdCard_.write(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&canFrame),
                                           sizeof(can::CanFrame)));
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

  const std::array<uint8_t, 9> version_ = {'N', 'F', 'R', '2', '6', '0', '0', '0', '\n'};

  inline static rtc::Rtc* activeRtc_ = nullptr;
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

  void init() override {}

  void run() override { sdCard_.handleFlush(); }

 private:
  sd::SdCard& sdCard_;
};

}  // namespace logger