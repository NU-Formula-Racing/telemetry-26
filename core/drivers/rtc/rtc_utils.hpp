#pragma once

#include <array>
#include <string_view>

#include "rtc.hpp"

namespace rtc::utils {

// convert __TIME__ ("HH:MM:SS") to RtcTime at compile-time
constexpr RtcTime parseCompilerTime(std::string_view timeStr) {
  RtcTime t{.hours = 0, .minutes = 0, .seconds = 0, .subseconds = 0};
  if (timeStr.size() >= 8) {
    t.hours = (timeStr.at(0) - '0') * 10 + (timeStr.at(1) - '0');
    t.minutes = (timeStr.at(3) - '0') * 10 + (timeStr.at(4) - '0');
    t.seconds = (timeStr.at(6) - '0') * 10 + (timeStr.at(7) - '0');
  }
  return t;
}

// use sakamoto's algorithm to calculate day of week (1=Monday ... 7=Sunday)
constexpr RtcWeekday calculateWeekday(uint16_t year, uint8_t month, uint8_t day) {
  constexpr std::array<uint8_t, 12> t = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (month < 3) {
    year -= 1;
  }
  uint8_t dow = (year + year / 4 - year / 100 + year / 400 + t.at(month - 1) + day) % 7;

  // sakamoto returns 0=Sunday, 1=Monday. We need 1=Monday ... 7=Sunday.
  return static_cast<RtcWeekday>((dow == 0) ? 7 : dow);
}

// Convert __DATE__ ("Mmm dd yyyy") to RtcDate at compile-time
constexpr RtcDate parseCompilerDate(std::string_view dateStr) {
  RtcDate d{.weekday = RtcWeekday::MONDAY, .month = 1, .day = 1, .year = 0};
  if (dateStr.size() >= 11) {
    // Parse Year (last two digits for STM32)
    d.year = (dateStr.at(9) - '0') * 10 + (dateStr.at(10) - '0');
    uint16_t fullYear = 2000 + d.year;

    // Parse Day (handle potential double space for single digit days e.g., "Feb  3 2026")
    d.day = (dateStr.at(4) == ' ' ? 0 : dateStr.at(4) - '0') * 10 + (dateStr.at(5) - '0');

    // Parse Month
    std::string_view m = dateStr.substr(0, 3);
    if (m == "Jan") {
      d.month = 1;
    } else if (m == "Feb") {
      d.month = 2;
    } else if (m == "Mar") {
      d.month = 3;
    } else if (m == "Apr") {
      d.month = 4;
    } else if (m == "May") {
      d.month = 5;
    } else if (m == "Jun") {
      d.month = 6;
    } else if (m == "Jul") {
      d.month = 7;
    } else if (m == "Aug") {
      d.month = 8;
    } else if (m == "Sep") {
      d.month = 9;
    } else if (m == "Oct") {
      d.month = 10;
    } else if (m == "Nov") {
      d.month = 11;
    } else if (m == "Dec") {
      d.month = 12;
    }

    d.weekday = calculateWeekday(fullYear, d.month, d.day);
  }
  return d;
}

}  // namespace rtc::utils
