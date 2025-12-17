#pragma once

namespace tasks {
class TaskManager;
}
class ILora;
class IUsb;
class ICan;
class ISd;
class IRtc;

namespace resources {

struct Context {
  tasks::TaskManager* taskManager = nullptr;
  ILora* lora = nullptr;
  IUsb* usb = nullptr;
  ICan* can = nullptr;
  ISd* sd = nullptr;
  IRtc* rtc = nullptr;
};

}  // namespace resources