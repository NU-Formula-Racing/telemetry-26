#include "main.h"

#include <FreeRTOS.h>

#include <cstring>

#include "app/base_station.hpp"
#include "drivers/lora/rfm95.hpp"
#include "drivers/usb/usb_stm32.hpp"
#include "lora.hpp"
#include "tasks/task.hpp"
#include "utils/utils.hpp"

extern "C" void BspInit(void);

class BlinkJob : public tasks::IJob {
 public:
  BlinkJob() = default;
  ~BlinkJob() override = default;

  // delete copy and move
  BlinkJob(const BlinkJob&) = delete;
  BlinkJob& operator=(const BlinkJob&) = delete;
  BlinkJob(BlinkJob&&) = delete;
  BlinkJob& operator=(BlinkJob&&) = delete;

  void init() override {
    HAL_GPIO_WritePin(LORA_STATUS_GPIO_Port, LORA_STATUS_Pin, GPIO_PIN_RESET);
  }

  void run() override { HAL_GPIO_TogglePin(LORA_STATUS_GPIO_Port, LORA_STATUS_Pin); }
};

int main() {
  BspInit();

  VERBOSITY(Verbosity::VERBOSE);

  // instantiate task manager
  static tasks::TaskManager taskMan;

  // instantiate drivers & interfaces STATICALLY
  static lora::rfm95::Rfm95 rfm95;
  static lora::Lora lora(rfm95);

  static usb::UsbStm32 usbDriver;
  static usb::Usb usb(usbDriver);

  // instantiate apps
  static wireless::Wireless wireless(lora);
  static base::BaseStation baseStation(wireless, usb);

  // setup tasks
  static BlinkJob blinkJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> blinkTask(
      tasks::TaskConfig{"BlinkTask", tasks::TaskPriority::STANDARD, 1000, blinkJob});

  static base::LoraReadJob loraReadJob(baseStation);
  static tasks::FreeRtosTask<tasks::TaskStackSize::XLARGE> loraReadTask(
      tasks::TaskConfig{"LoraReadTask", tasks::TaskPriority::STANDARD, 100, loraReadJob});

  static base::UsbWriteJob usbWriteJob(baseStation);
  static tasks::FreeRtosTask<tasks::TaskStackSize::LARGE> usbWriteTask(
      tasks::TaskConfig{"UsbWriteTask", tasks::TaskPriority::STANDARD, 100, usbWriteJob});

  // start all tasks
  taskMan.addTask(std::move(blinkTask));
  taskMan.addTask(std::move(loraReadTask));
  taskMan.addTask(std::move(usbWriteTask));

  taskMan.startAllTasks();
  vTaskStartScheduler();

  while (true) {
  }
}