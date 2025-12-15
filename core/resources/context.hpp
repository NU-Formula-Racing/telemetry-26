#pragma once

namespace tasks {
class TaskManager;
}
class ILoraInterface;
class IUsbInterface;
class ICanInterface;
class ISdInterface;
class IRtcInterface;

namespace resources {

struct Context {
  tasks::TaskManager* taskManager = nullptr;
  ILoraInterface* lora = nullptr;
  IUsbInterface* usb = nullptr;
  ICanInterface* can = nullptr;
  ISdInterface* sd = nullptr;
  IRtcInterface* rtc = nullptr;
};

}  // namespace resources