#pragma once

#include "wireless.hpp"

namespace base {

class BaseStation {
 public:
  BaseStation(wireless::Wireless& wireless /*, Usb& usb*/) : wireless_(wireless) /*, usb_(usb)*/ {}
  ~BaseStation() = default;

  BaseStation(const BaseStation&) = delete;
  BaseStation& operator=(const BaseStation&) = delete;
  BaseStation(BaseStation&&) = delete;
  BaseStation& operator=(BaseStation&&) = delete;

  void init() {
    wireless_.init();
    // usb_.init();
  }

 private:
  wireless::Wireless& wireless_;
  // Usb& usb_;
};

}  // namespace base