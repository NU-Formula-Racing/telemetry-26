#include "app/app.hpp"

extern "C" void BspInit(void);
// get STM HAL peripheral handlers
// extern SPI_HandleTypeDef hspi2;

int main() {
  BspInit();

  // init freertos stuff
  // osKernelInitialize();

  // instantiate drivers - lora, usb
  // pass in HAL dependencies
  // static StmLora loraDriver(&hspi2);
  // static StmUsb usbDriver;

  // create registry
  // TelemetryRegistry registry;
  // registry.lora = &loraDriver;
  // registry.usb = &usbDriver;
  // registry.can = nullptr;
  // registry.sd = nullptr;

  while (true) {
  }

  return 0;
}