#pragma once

#include <cstdint>

#include "can_msgs.hpp"
#include "drivers/can/can.hpp"
#include "drivers/rtc/rtc.hpp"
#include "drivers/rtc/rtc_utils.hpp"
#include "drivers/sd/sd.hpp"
#include "odometer.hpp"
#include "tasks/job.hpp"

namespace logger {

#pragma pack(push, 1)
struct LogHeader {
  std::array<uint8_t, 9> version;  // NFR26000\n -> NFR26 v0.0.0
  rtc::RtcDate startDate;
  rtc::RtcTime startTime;
};
#pragma pack(pop)

enum class LoggingStatus : uint8_t {
  OK = 0,
  CARD_NOT_DETECTED = 1,
  MOUNT_FILESYSTEM_ERROR = 2,
  CARD_FULL = 3,
  SDIO_COMMS_ERROR = 4,
};

struct LoggerStatus {
  uint16_t logFileIndex = 0;
  LoggingStatus loggingStatus = LoggingStatus::OK;
};

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

    // recover miles driven from most recent log file
    std::string mostRecentDir = sdCard_.getMostRecentValidDir();
    std::string lastLog{};
    if (!mostRecentDir.empty()) {
      lastLog = sdCard_.getLastLogFile(mostRecentDir);
    }
    odometer_.init(recoverMilesDriven(lastLog));

    // open new log file for this session
    sdCard_.openRollingLogFile(rtc_.getDate().toString());
    status_.logFileIndex = sdCard_.getOpenFileIndex();

    // log header
    LogHeader header{};
    header.version = version_;
    header.startDate = rtc_.getDate();
    header.startTime = rtc_.getTime();
    sdCard_.write(
        std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&header), sizeof(LogHeader)));

    initialized_ = true;
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

    if (!initialized_) {
      return;
    }

    sdCard_.write(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&canFrame),
                                           sizeof(can::CanFrame)));
  }

  float recoverMilesDriven(const std::string& lastLog) {
    float milesRecovered = 0.0F;
    if (!lastLog.empty()) {
      sd::SdResult res =
          sdCard_.openFile(lastLog, sd::SdFileMode::READ | sd::SdFileMode::OPEN_EXISTING);
      if (res == sd::SdResult::OK) {
        uint32_t fileSize = sdCard_.getOpenFileSize();
        uint32_t chunkSize = std::min<uint32_t>(fileSize, 4096U);

        sdCard_.seek(fileSize - chunkSize);  // seek to last chunk of file

        // read last chunk of file
        etl::vector<uint8_t, 4096> lastFrames{};
        lastFrames.resize(chunkSize);
        sdCard_.read(std::span(lastFrames));

        // search backwards for last odometer CanFrame
        for (int i = lastFrames.size() - sizeof(can::CanFrame); i >= 0; i--) {
          std::array<uint8_t, sizeof(can::CanFrame)> chunk{};
          std::copy_n(lastFrames.begin() + i, sizeof(can::CanFrame), chunk.begin());
          auto f = std::bit_cast<can::CanFrame>(chunk);

          if (f.id == remote::canmsgs::TelemetryOdometer::ID) {
            //  found the odometer frame, extract miles driven
            auto odom = can::decode<remote::canmsgs::TelemetryOdometer>(f);
            // milesRecovered = odom.milesDriven;
            milesRecovered = 14.61F;
            break;
          }
        }
      }
      sdCard_.closeFile();  // this might do literally nothing
    }
    return milesRecovered;
  }

  void updateOdometer(const int16_t motorRpm, const uint32_t timestamp) {
    odometer_.updateMilesDriven(motorRpm, timestamp);
  }

  float getMilesDriven() const { return odometer_.getMilesDriven(); }

  rtc::RtcTime getRtcTime() const { return rtc_.getTime(); }

  rtc::RtcDate getRtcDate() const { return rtc_.getDate(); }

  LoggerStatus getStatus() const { return status_; }

 private:
  static uint32_t provideFatTime() {
    const rtc::RtcTime time = activeRtc_->getTime();
    const rtc::RtcDate date = activeRtc_->getDate();

    return sd::SdCard::packFatTime(date.year, date.month, date.day, time.hours, time.minutes,
                                   time.seconds);
  }

  sd::SdCard& sdCard_;
  rtc::Rtc& rtc_;
  odometer::Odometer odometer_;

  const std::array<uint8_t, 9> version_ = {'N', 'F', 'R', '2', '6', '0', '0', '0', '\n'};

  inline static rtc::Rtc* activeRtc_ = nullptr;

  volatile bool initialized_ = false;

  LoggerStatus status_;
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