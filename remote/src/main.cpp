#include "../bsp/Core/Inc/main.h"

#include <cstring>

#include "../bsp/USB_DEVICE/App/usb_device.h"
#include "../bsp/USB_DEVICE/App/usbd_cdc_if.h"
#include "app/app.hpp"

extern "C" void BspInit(void);
int main() {
  BspInit();

  while (true) {
    // 1. Blink the LED so we know the board is alive
    HAL_GPIO_TogglePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin);

    // 2. Define the message
    const char* msg = "Hello World\r\n";

    // 3. Transmit over USB
    // CDC_Transmit_FS takes a uint8_t pointer and the length of the data
    CDC_Transmit_FS((uint8_t*)msg, strlen(msg));

    // 4. Wait
    HAL_Delay(1000);
  }
}