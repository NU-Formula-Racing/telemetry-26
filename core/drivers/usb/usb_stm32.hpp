#pragma once

#include "projdefs.h"
#include "usb.hpp"

namespace usb {

class UsbStm32 : public IUsbDriver {
  void init() override {
    // check if cube code is getting called in main
  }

  void write(std::span<const uint8_t> data) override {
    if (data.empty()) {
      return;
    }

    size_t retries = 0;
    while (CDC_Transmit_FS(const_cast<uint8_t*>(data.data()), data.size()) != 0 /*USBD_OK*/) {
      // block until transmission is successful, or timeout eventually

      retries++;
      if (retries > 50) {
        break;
      }
      // yield
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
};

}  // namespace usb