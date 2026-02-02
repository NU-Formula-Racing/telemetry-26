#pragma once

namespace lora {

class ILora {
 public:
  ILora() = default;

  // basic functionality needed:
  // init, configure radio (frequency, power, spreading factor, bandwidth, coding rate)
  // put radio into sleep, standby, tx, rx modes
  // enable/disable crc
  // send
  // receive (continuous, single)

 private:
};

}  // namespace lora