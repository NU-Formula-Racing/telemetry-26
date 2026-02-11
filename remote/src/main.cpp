#include "main.h"

#include <FreeRTOS.h>

#include <cstdint>
#include <cstring>

#include "app/logger.hpp"
#include "drivers/rtc/rtc_stm32.hpp"
#include "drivers/sd/sd_stm32.hpp"
#include "resources/context.hpp"
#include "stm32f4xx_hal_can.h"
#include "tasks/task.hpp"
#include "utils/utils.hpp"

// TODO: //
// read from CAN in ISR, task
// write to CAN in task
// write CAN driver, make PR

// not needed for EI MVP:
// hardware timer instead of RTC for more accurate logging timestamps
// send lora
// receive lora
// write lora driver
// GPS
// read from sd
// clean up long ahh includes
// change cmake to lint regardless of platform
// phase out STM main.c and init code in drivers, comment out main.c in CMakeLists

extern "C" void BspInit(void);
// get STM HAL peripheral handlers
// extern SPI_HandleTypeDef hspi2;
extern CAN_HandleTypeDef hcan1;

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
    HAL_GPIO_WritePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin, GPIO_PIN_RESET);  // turn led off
  }

  void run() override {
    DEBUG_OUT("blinkJob", MAGENTA, "blinkJob\r\n");
    HAL_GPIO_TogglePin(SD_STATUS_GPIO_Port, SD_STATUS_Pin);
  }
};

class SdPeriodicSyncJob : public tasks::IJob {
 public:
  SdPeriodicSyncJob(sd::SdCard& sdCard) : sdCard_(sdCard) {}
  ~SdPeriodicSyncJob() override = default;

  // delete copy and move
  SdPeriodicSyncJob(const SdPeriodicSyncJob&) = delete;
  SdPeriodicSyncJob& operator=(const SdPeriodicSyncJob&) = delete;
  SdPeriodicSyncJob(SdPeriodicSyncJob&&) = delete;
  SdPeriodicSyncJob& operator=(SdPeriodicSyncJob&&) = delete;

  void init() override {}

  void run() override {
    sdCard_.periodicSync();
    DEBUG_OUT("SdPeriodicSyncJob", CYAN, "Flushed SD card buffers\r\n");
  }

 private:
  sd::SdCard& sdCard_;
};

// task to periodically read from CAN

// task to periodically write to CAN
class CanTxJob : public tasks::IJob {
 public:
  CanTxJob(CAN_HandleTypeDef& hcan) : hcan_(hcan) {};
  ~CanTxJob() override = default;

  // delete copy and move
  CanTxJob(const CanTxJob&) = delete;
  CanTxJob& operator=(const CanTxJob&) = delete;
  CanTxJob(CanTxJob&&) = delete;
  CanTxJob& operator=(CanTxJob&&) = delete;

  void init() override {
    canfilterconfig_.FilterActivation = CAN_FILTER_ENABLE;
    canfilterconfig_.FilterBank = 0;
    canfilterconfig_.FilterFIFOAssignment = CAN_FILTER_FIFO0;

    canfilterconfig_.FilterIdHigh = 0;
    canfilterconfig_.FilterIdLow = 0;

    canfilterconfig_.FilterMaskIdHigh = 0;
    canfilterconfig_.FilterMaskIdLow = 0;

    canfilterconfig_.FilterMode = CAN_FILTERMODE_IDMASK;
    canfilterconfig_.FilterScale = CAN_FILTERSCALE_32BIT;

    canfilterconfig_.SlaveStartFilterBank = 14;

    auto status = HAL_CAN_ConfigFilter(&hcan_, &canfilterconfig_);
    if (status != HAL_OK) {
      while (true) {
        ERROR("CanTxJob", "Failed to configure CAN filter, status: ", std::to_string(status),
              "\r\n");
        HAL_Delay(100);
      }
    }

    txHeader_.IDE = CAN_ID_STD;
    txHeader_.StdId = 0x520;
    txHeader_.RTR = CAN_RTR_DATA;
    txHeader_.DLC = 2;

    txData_.at(0) = 50;
    txData_.at(1) = counter_;

    HAL_CAN_ActivateNotification(&hcan_, CAN_IT_RX_FIFO0_MSG_PENDING);

    HAL_CAN_Start(&hcan_);
  }

  void run() override {
    auto status = HAL_CAN_AddTxMessage(&hcan_, &txHeader_, txData_.data(), &txMailbox_);

    if (status != HAL_OK) {
      ERROR("CanTxJob", "Failed to send CAN message, status: ", std::to_string(status), "\r\n");
    } else {
      DEBUG_OUT("CanTxJob", BLUE, "Sent CAN message with ID ", std::to_string(txHeader_.StdId),
                " and data ", std::to_string(txData_.at(0)), " ", std::to_string(txData_.at(1)),
                "\r\n");
    }
    counter_++;
    txData_.at(1) = counter_;
  }

 private:
  CAN_FilterTypeDef canfilterconfig_;
  CAN_HandleTypeDef& hcan_;
  CAN_TxHeaderTypeDef txHeader_;
  std::array<uint8_t, 8> txData_;
  uint32_t txMailbox_;
  uint8_t counter_ = 0;
};

CAN_RxHeaderTypeDef rxHeader;
std::array<uint8_t, 8> rxData;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan) {
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData.data()) != HAL_OK) {
    ERROR("CAN Rx Callback", "Failed to receive CAN message\r\n");
  } else {
    DEBUG_OUT("CAN Rx Callback", YELLOW, "Received CAN message with ID ",
              std::to_string(rxHeader.StdId), " and data ", std::to_string(rxData.at(0)), " ",
              std::to_string(rxData.at(1)), "\r\n");
  }
}

// class CanRxJob : public tasks::IJob {
//  public:
//   CanRxJob(CAN_HandleTypeDef& hcan) : hcan_(hcan) {};
//   ~CanRxJob() override = default;
//
//   // delete copy and move
//   CanRxJob(const CanRxJob&) = delete;
//   CanRxJob& operator=(const CanRxJob&) = delete;
//   CanRxJob(CanRxJob&&) = delete;
//   CanRxJob& operator=(CanRxJob&&) = delete;
//
//   void init() override {}
//
//   void run() override {
//     CAN_RxHeaderTypeDef rxHeader;
//     std::array<uint8_t, 8> rxData;
//     auto status = HAL_CAN_GetRxMessage(&hcan_, CAN_RX_FIFO0, &rxHeader, rxData.data());
//
//     if (status == HAL_OK) {
//       DEBUG_OUT("CanRxJob", GREEN, "Received CAN message with ID ",
//       std::to_string(rxHeader.StdId),
//                 " and data ", std::to_string(rxData.at(0)), " ", std::to_string(rxData.at(1)),
//                 "\r\n");
//     }
//   }
//
//  private:
//   CAN_HandleTypeDef& hcan_;
// };

int main() {
  BspInit();

  VERBOSITY(Verbosity::VERBOSE);

  // instantiate task manager
  static tasks::TaskManager taskMan;

  // instantiate drivers & interfaces STATICALLY
  static sd::Stm32SdDriver sdDriver;
  static sd::SdCard sd(sdDriver);
  static rtc::Stm32RtcDriver rtcDriver;
  static rtc::Rtc rtc(rtcDriver);

  // create and populate context
  static resources::Context ctx;
  ctx.taskManager = &taskMan;
  // ctx.lora = &lora;
  // ctx.usb = nullptr;
  // ctx.can = &can;
  ctx.sd = &sd;
  ctx.rtc = &rtc;

  // instantiate apps
  static logger::Logger logger(sd, rtc);

  // setup tasks
  static BlinkJob blinkJob;
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> blinkTask(
      tasks::TaskConfig{"BlinkTask", tasks::TaskPriority::STANDARD, 1000, blinkJob});

  static logger::LoggerJob loggerJob(logger);
  static tasks::FreeRtosTask<tasks::TaskStackSize::MEDIUM> loggerTask(
      tasks::TaskConfig{"LoggerTask", tasks::TaskPriority::STANDARD, 10, loggerJob});

  static SdPeriodicSyncJob sdSyncJob(*ctx.sd);
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> sdPeriodicSyncTask(
      tasks::TaskConfig{"SdSyncTask", tasks::TaskPriority::STANDARD, 500, sdSyncJob});

  static CanTxJob canTxJob(hcan1);
  static tasks::FreeRtosTask<tasks::TaskStackSize::SMALL> canTxTask(
      tasks::TaskConfig{"CanTxTask", tasks::TaskPriority::STANDARD, 500, canTxJob});

  // start all tasks
  taskMan.addTask(std::move(blinkTask));
  taskMan.addTask(std::move(loggerTask));
  taskMan.addTask(std::move(sdPeriodicSyncTask));
  taskMan.addTask(std::move(canTxTask));
  taskMan.startAllTasks();
  vTaskStartScheduler();

  while (true) {
  }
}