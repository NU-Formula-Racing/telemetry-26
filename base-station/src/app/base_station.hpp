#pragma once

#include "tasks/job.hpp"
#include "utils/utils.hpp"
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

  void processIncomingPackets() {
    auto packet = wireless_.receive();
    if (packet.empty()) {
      return;
    }

    // usb_.write(packet);
    DEBUG_OUT("BaseStation", CYAN, "Received packet: ", packet.toString(), "\r\n");
  }

 private:
  wireless::Wireless& wireless_;
  // Usb& usb_;
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

}  // namespace base