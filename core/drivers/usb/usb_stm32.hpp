#pragma once

#include "usb.hpp"

namespace usb {

class Stm32UsbDriver : public IUsbDriver {
 public:
  Stm32UsbDriver() = default;
  ~Stm32UsbDriver() override = default;

  // delete copy and move
  Stm32UsbDriver(const Stm32UsbDriver&) = delete;
  Stm32UsbDriver& operator=(const Stm32UsbDriver&) = delete;
  Stm32UsbDriver(Stm32UsbDriver&&) = delete;
  Stm32UsbDriver& operator=(Stm32UsbDriver&&) = delete;

  void init() override {}

  bool write(const std::span < const uint8_t& data) override { return false; }

 private:
};

}  // namespace usb