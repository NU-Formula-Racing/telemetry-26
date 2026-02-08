#pragma once

#include "rtc.hpp"
#include "stm32f4xx_hal_rtc.h"

namespace rtc {

class Stm32RtcDriver : public IRtcDriver {
 public:
  // Stm32RtcDriver(RTC_HandleTypeDef& hrtc) : hrtc_(hrtc) {}
  Stm32RtcDriver() = default;
  ~Stm32RtcDriver() override = default;

  RtcStatus init() override {
    hrtc_.Instance = RTC;
    hrtc_.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc_.Init.AsynchPrediv = 127;
    hrtc_.Init.SynchPrediv = 255;
    hrtc_.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc_.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc_.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    status_ = HAL_RTC_Init(&hrtc_);

    // check backup register to see if RTC has already been initialized, if not set a time and
    // date 1/1/2000 00:00:00
    // if (HAL_RTCEx_BKUPRead(&hrtc_, RTC_BKP_DR1) != 0x2345) {
    //   RtcTime t;
    //   t.hours = 0;
    //   t.minutes = 0;
    //   t.seconds = 0;
    //   t.subseconds = 0;
    //   setTime(t);

    //   RtcDate d;
    //   d.weekday = RTC_WEEKDAY_SUNDAY;
    //   d.day = 1;
    //   d.month = 1;
    //   d.year = 0;
    //   setDate(d);
    // }

    return status_ == HAL_OK ? RtcStatus::OK : RtcStatus::ERROR;
  }

  RtcStatus setTime(const RtcTime& time) override {
    // only set time if it hasn't already been set previously, to avoid overwriting on every reset
    // (since RTC
    // if (HAL_RTCEX_BKUPRead(&hrtc_, RTC_BKP_DR1) != 0x2345) {
    RTC_TimeTypeDef sTime;
    sTime.Hours = time.hours;
    sTime.Minutes = time.minutes;
    sTime.Seconds = time.seconds;
    sTime.SubSeconds = time.subseconds;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    status_ = HAL_RTC_SetTime(&hrtc_, &sTime, RTC_FORMAT_BIN);

    return status_ == HAL_OK ? RtcStatus::OK : RtcStatus::ERROR;
  }

  RtcStatus setDate(const RtcDate& date) override {
    RTC_DateTypeDef sDate;
    sDate.WeekDay = static_cast<uint8_t>(date.weekday);
    sDate.Month = date.month;
    sDate.Date = date.day;
    sDate.Year = date.year;

    status_ = HAL_RTC_SetDate(&hrtc_, &sDate, RTC_FORMAT_BIN);

    // write to backup register to indicate RTC has been initialized, so we don't overwrite on every
    // reset
    // HAL_RTCEx_BKUPWrite(&hrtc_, RTC_BKP_DR1, 0x2345);

    return status_ == HAL_OK ? RtcStatus::OK : RtcStatus::ERROR;
  }

  RtcTime getTime() override {
    HAL_RTC_GetTime(&hrtc_, &time_, RTC_FORMAT_BIN);
    return RtcTime{time_.Hours, time_.Minutes, time_.Seconds, time_.SubSeconds};
  }

  RtcDate getDate() override {
    HAL_RTC_GetDate(&hrtc_, &date_, RTC_FORMAT_BIN);
    return RtcDate{static_cast<RtcWeekday>(date_.WeekDay), date_.Date, date_.Month, date_.Year};
  }

 private:
  RTC_HandleTypeDef hrtc_;
  HAL_StatusTypeDef status_;
  RTC_TimeTypeDef time_;
  RTC_DateTypeDef date_;
  // uint32_t backupMagicNum_;
};
}  // namespace rtc