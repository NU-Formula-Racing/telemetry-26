#pragma once

#include "drivers/usb/usb.hpp"
#include "tasks/job.hpp"
#include "utils/utils.hpp"
#include "wireless.hpp"

namespace base {

class BaseStation {
 public:
  BaseStation(wireless::Wireless& wireless, usb::Usb& usb) : wireless_(wireless), usb_(usb) {}
  ~BaseStation() = default;

  BaseStation(const BaseStation&) = delete;
  BaseStation& operator=(const BaseStation&) = delete;
  BaseStation(BaseStation&&) = delete;
  BaseStation& operator=(BaseStation&&) = delete;

  void init() {
    wireless_.init();
    usb_.init();
  }

  void processIncomingPackets() {
    auto packet = wireless_.receive();
    if (packet.empty()) {
      return;
    }

    std::array<uint8_t, sizeof(lora::RxPacket)> packetBytes;
    std::memcpy(packetBytes.data(), &packet, sizeof(packetBytes));
    usb_.write(std::span(packetBytes.data(), sizeof(lora::RxPacket)));
    // DEBUG_OUT("BaseStation", CYAN, "Received packet of size ", std::to_string(packet.size()), ":
    // ",
    //           packet.toString(), "\r\n");
  }

  void flushUsb() { usb_.flush(); }

 private:
  wireless::Wireless& wireless_;
  usb::Usb& usb_;
};

class LoraReadJob : public tasks::IJob {
 public:
  LoraReadJob(BaseStation& baseStation) : baseStation_(baseStation) {}
  ~LoraReadJob() override = default;

  // delete copy and move
  LoraReadJob(const LoraReadJob&) = delete;
  LoraReadJob& operator=(const LoraReadJob&) = delete;
  LoraReadJob(LoraReadJob&&) = delete;
  LoraReadJob& operator=(LoraReadJob&&) = delete;

  void init() override { baseStation_.init(); }

  void run() override { baseStation_.processIncomingPackets(); }

 private:
  BaseStation& baseStation_;
};

class UsbWriteJob : public tasks::IJob {
 public:
  UsbWriteJob(BaseStation& baseStation) : baseStation_(baseStation) {}
  ~UsbWriteJob() override = default;

  // delete copy and move
  UsbWriteJob(const UsbWriteJob&) = delete;
  UsbWriteJob& operator=(const UsbWriteJob&) = delete;
  UsbWriteJob(UsbWriteJob&&) = delete;
  UsbWriteJob& operator=(UsbWriteJob&&) = delete;

  void init() override {}

  void run() override { baseStation_.flushUsb(); }

 private:
  BaseStation& baseStation_;
};

}  // namespace base