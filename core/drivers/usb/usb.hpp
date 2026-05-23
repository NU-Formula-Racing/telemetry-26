#pragma once

#include <cstdint>
#include <span>

namespace usb {

class IUsbDriver {
 public:
  virtual ~IUsbDriver() = default;

  virtual void init() = 0;

  virtual bool write(const std::span<const uint8_t>& data) = 0;
};

class Usb {
 public:
  Usb(IUsbDriver& driver) : driver_(driver) {}
  ~Usb() = default;

  Usb(const Usb&) = delete;
  Usb& operator=(const Usb&) = delete;
  Usb(Usb&&) = delete;
  Usb& operator=(Usb&&) = delete;

  void init() { driver_.init(); }

  bool write(const std::span<const uint8_t>& data) { return driver_.write(data); }

 private:
  IUsbDriver& driver_;
};

}  // namespace usb