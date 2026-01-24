#pragma once

namespace usb {

class IUsb {
 public:
  IUsb() = default;

  // basic functionality needed:
  // init, configure usb (device descriptors, endpoints)
  // send data
  // receive data

  // id like to do something with debug macros and map them to usb methods - utils file, 322 type

 private:
};

}  // namespace usb