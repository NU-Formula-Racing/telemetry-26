#pragma once

#include "rtc.hpp"
#include "stm32f4xx_hal_rtc.h"

namespace rtc {

class Stm32RtcDriver : public IRtcDriver {
 public:
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

    return status_ == HAL_OK ? RtcStatus::OK : RtcStatus::ERROR;
  }

  RtcStatus setTime(const RtcTime& time, uint32_t backupMagicNum) override {
    // only set time if it hasn't already been set previously, to avoid overwriting on every reset
    if (HAL_RTCEx_BKUPRead(&hrtc_, RTC_BKP_DR1) != backupMagicNum) {
      RTC_TimeTypeDef sTime;
      sTime.Hours = time.hours;
      sTime.Minutes = time.minutes;
      sTime.Seconds = time.seconds;
      sTime.SubSeconds = msToSubseconds_(time.subseconds);
      sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
      sTime.StoreOperation = RTC_STOREOPERATION_RESET;

      status_ = HAL_RTC_SetTime(&hrtc_, &sTime, RTC_FORMAT_BIN);
      HAL_RTCEx_BKUPWrite(&hrtc_, RTC_BKP_DR1, backupMagicNum);
    } else {
      status_ = HAL_OK;
    }

    return status_ == HAL_OK ? RtcStatus::OK : RtcStatus::ERROR;
  }

  RtcStatus setDate(const RtcDate& date, uint32_t backupMagicNum) override {
    // only set date if it hasn't already been set previously, to avoid overwriting on every reset
    if (HAL_RTCEx_BKUPRead(&hrtc_, RTC_BKP_DR2) != backupMagicNum) {
      RTC_DateTypeDef sDate;
      sDate.WeekDay = static_cast<uint8_t>(date.weekday);
      sDate.Month = date.month;
      sDate.Date = date.day;
      sDate.Year = date.year;

      status_ = HAL_RTC_SetDate(&hrtc_, &sDate, RTC_FORMAT_BIN);
      HAL_RTCEx_BKUPWrite(&hrtc_, RTC_BKP_DR2, backupMagicNum);
    } else {
      status_ = HAL_OK;
    }

    return status_ == HAL_OK ? RtcStatus::OK : RtcStatus::ERROR;
  }

  RtcTime getTime() override {
    HAL_RTC_GetTime(&hrtc_, &time_, RTC_FORMAT_BIN);
    return RtcTime{time_.Hours, time_.Minutes, time_.Seconds, subsecondsToMs_(time_.SubSeconds)};
  }

  RtcDate getDate() override {
    HAL_RTC_GetDate(&hrtc_, &date_, RTC_FORMAT_BIN);
    return RtcDate{static_cast<RtcWeekday>(date_.WeekDay), date_.Month, date_.Date, date_.Year};
  }

 private:
  uint32_t subsecondsToMs_(uint8_t subseconds) {
    // subseconds reg counts down from SynchPrediv (255) -> 0, so we need to flip
    const uint32_t flippedSubseconds = hrtc_.Init.SynchPrediv - subseconds;
    // convert to ms (ie. 255 subseconds = 1 second = 1000 ms)
    return (flippedSubseconds * 1000) / (hrtc_.Init.SynchPrediv + 1);
  }

  uint32_t msToSubseconds_(uint32_t ms) {
    // convert ms to subseconds value
    const uint32_t subseconds = (ms * (hrtc_.Init.SynchPrediv + 1)) / 1000;
    // flip for countdown reg
    return hrtc_.Init.SynchPrediv - subseconds;
  }

  RTC_HandleTypeDef hrtc_;
  HAL_StatusTypeDef status_;
  RTC_TimeTypeDef time_;
  RTC_DateTypeDef date_;
};

}  // namespace rtc