#pragma once

#include <cstdint>
#include <string>

namespace rtc {

// TODO: better statuses
enum class RtcStatus : uint8_t { OK, ERROR };

enum class RtcWeekday : uint8_t {
  MONDAY = 0x01,
  TUESDAY = 0x02,
  WEDNESDAY = 0x03,
  THURSDAY = 0x04,
  FRIDAY = 0x05,
  SATURDAY = 0x06,
  SUNDAY = 0x07
};

struct RtcTime {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint32_t subseconds;

  std::string toString() const {
    return std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(seconds) +
           ":" + std::to_string(subseconds);
  }
};

struct RtcDate {
  RtcWeekday weekday;
  uint8_t month;
  uint8_t day;
  uint8_t year;  // last 2 digits of year (ie. 0-99, representing 2000-2099)

  std::string toString() const {
    return std::to_string(month) + "/" + std::to_string(day) + "/" + std::to_string(year);
  }
};

class IRtcDriver {
 public:
  IRtcDriver() = default;
  virtual ~IRtcDriver() = default;

  // delete copy and move
  IRtcDriver(const IRtcDriver&) = delete;
  IRtcDriver& operator=(const IRtcDriver&) = delete;
  IRtcDriver(IRtcDriver&&) = delete;
  IRtcDriver& operator=(IRtcDriver&&) = delete;

  // initialize RTC hardware
  virtual RtcStatus init() = 0;

  // set time and date on RTC
  virtual RtcStatus setTime(const RtcTime& time) = 0;
  virtual RtcStatus setDate(const RtcDate& date) = 0;

  // get current time and date from RTC
  virtual RtcTime getTime() = 0;
  virtual RtcDate getDate() = 0;
};

// TODO: possibly move init stuff from stm main.c to here, alarm/interrupt
class Rtc {
 public:
  Rtc(IRtcDriver& driver) : driver_(driver) {}
  ~Rtc() = default;

  // delete copy and move
  Rtc(const Rtc&) = delete;
  Rtc& operator=(const Rtc&) = delete;
  Rtc(Rtc&&) = delete;
  Rtc& operator=(Rtc&&) = delete;

  RtcStatus init() { return driver_.init(); }

  RtcStatus setTime(const RtcTime& time, bool newTime) {
    RtcStatus s = RtcStatus::OK;
    if (newTime) {
      s = driver_.setTime(time);
    }
    return s;
  }

  RtcStatus setDate(const RtcDate& date, bool newDate) {
    RtcStatus s = RtcStatus::OK;
    if (newDate) {
      s = driver_.setDate(date);
    }
    return s;
  }

  RtcTime getTime() { return driver_.getTime(); }

  RtcDate getDate() { return driver_.getDate(); }

 private:
  IRtcDriver& driver_;
};

}  // namespace rtc