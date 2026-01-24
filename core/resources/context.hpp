#pragma once

namespace tasks {
class TaskManager;
}
namespace lora {
class ILora;
}
namespace usb {
class IUsb;
}
namespace can {
class ICan;
}
namespace sd {
class ISd;
}
namespace rtc {
class IRtc;
}

namespace resources {

struct Context {
  tasks::TaskManager* taskManager = nullptr;
  lora::ILora* lora = nullptr;
  usb::IUsb* usb = nullptr;
  can::ICan* can = nullptr;
  sd::ISd* sd = nullptr;
  rtc::IRtc* rtc = nullptr;
};

}  // namespace resources