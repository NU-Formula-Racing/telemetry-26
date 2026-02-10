#pragma once
#include "drivers/rtc/rtc.hpp"
#include "drivers/sd/sd.hpp"
#include "tasks/task.hpp"

namespace resources {

struct Context {
  tasks::TaskManager* taskManager = nullptr;
  // lora::ILora* lora = nullptr;
  // usb::IUsb* usb = nullptr;
  // can::ICan* can = nullptr;
  sd::SdCard* sd = nullptr;
  rtc::Rtc* rtc = nullptr;
};

}  // namespace resources