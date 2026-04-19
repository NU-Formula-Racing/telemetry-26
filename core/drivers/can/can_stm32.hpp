#pragma once

#include <array>

#include "can.hpp"
#include "stm32f4xx_hal_can.h"

namespace can {

class Stm32CanDriver : public ICanDriver {
 public:
  Stm32CanDriver() {
    instance_ = this;  // set static instance ptr to this instance for use in static ISR handler
  }
  ~Stm32CanDriver() override = default;

  // delete copy and move
  Stm32CanDriver(const Stm32CanDriver&) = delete;
  Stm32CanDriver& operator=(const Stm32CanDriver&) = delete;
  Stm32CanDriver(Stm32CanDriver&&) = delete;
  Stm32CanDriver& operator=(Stm32CanDriver&&) = delete;

  CanStatus init() override {
    hcan_.Instance = CAN1;
    hcan_.Init.Prescaler = 4;
    hcan_.Init.Mode = CAN_MODE_NORMAL;
    hcan_.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan_.Init.TimeSeg1 = CAN_BS1_16TQ;
    hcan_.Init.TimeSeg2 = CAN_BS2_4TQ;
    hcan_.Init.TimeTriggeredMode = DISABLE;
    hcan_.Init.AutoBusOff = DISABLE;
    hcan_.Init.AutoWakeUp = DISABLE;
    hcan_.Init.AutoRetransmission = DISABLE;
    hcan_.Init.ReceiveFifoLocked = DISABLE;
    hcan_.Init.TransmitFifoPriority = DISABLE;
    HAL_StatusTypeDef status = HAL_CAN_Init(&hcan_);

    // setup filters
    // listening to all messages for now
    // TODO: make filtering configurable
    canFilterConfig_.FilterActivation = CAN_FILTER_ENABLE;
    canFilterConfig_.FilterBank = 0;
    canFilterConfig_.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    canFilterConfig_.FilterIdHigh = 0;
    canFilterConfig_.FilterIdLow = 0;
    canFilterConfig_.FilterMaskIdHigh = 0;
    canFilterConfig_.FilterMaskIdLow = 0;
    canFilterConfig_.FilterMode = CAN_FILTERMODE_IDMASK;
    canFilterConfig_.FilterScale = CAN_FILTERSCALE_32BIT;
    canFilterConfig_.SlaveStartFilterBank = 14;

    status = HAL_CAN_ConfigFilter(&hcan_, &canFilterConfig_);

    // always enabling RX0 interrupt for now, TODO: make configurable
    HAL_CAN_ActivateNotification(&hcan_, CAN_IT_RX_FIFO0_MSG_PENDING);

    HAL_CAN_Start(&hcan_);

    return status == HAL_OK ? CanStatus::OK : CanStatus::ERROR;
  }

  virtual CanStatus send(const CanFrame& frame) override {
    // populate header
    CAN_TxHeaderTypeDef txHeader;
    txHeader.IDE = (frame.idType == CanIdType::STANDARD) ? CAN_ID_STD : CAN_ID_EXT;
    txHeader.StdId = frame.id & STD_ID_MASK;
    // txHeader.ExtId = frame.id;  // HAL automatically uses ExtId if IDE is set to CAN_ID_EXT
    txHeader.RTR = CAN_RTR_DATA;  // only supporting data frames
    txHeader.DLC = frame.dlc;

    auto status = HAL_CAN_AddTxMessage(&hcan_, &txHeader, frame.data.data(), &txMailbox_);

    return status == HAL_OK ? CanStatus::OK : CanStatus::ERROR;
  }

  void registerRxCallback(CanRxCallback cb) override { rxCallback_ = cb; }

  // public helper called by global C ISR
  void handleRxInterrupt() {
    CAN_RxHeaderTypeDef rxHeader;
    CanFrame frame;

    auto status = HAL_CAN_GetRxMessage(&hcan_, CAN_RX_FIFO0, &rxHeader, frame.data.data());
    if (status == HAL_OK) {
      // populate frame
      frame.timestamp = HAL_GetTick();
      frame.idType = (rxHeader.IDE == CAN_ID_STD) ? CanIdType::STANDARD : CanIdType::EXTENDED;
      frame.id = (rxHeader.IDE == CAN_ID_STD) ? rxHeader.StdId & STD_ID_MASK
                                              : rxHeader.ExtId & EXT_ID_MASK;
      frame.dlc = rxHeader.DLC;

      if (rxCallback_ != nullptr) {
        rxCallback_(frame);
      }
    }
  }

  static Stm32CanDriver* instance() { return instance_; }

 private:
  CAN_HandleTypeDef hcan_;
  CAN_FilterTypeDef canFilterConfig_;
  uint32_t txMailbox_;

  CanRxCallback rxCallback_ = nullptr;

  // static ptr to the active driver instance
  inline static Stm32CanDriver* instance_ = nullptr;
};

}  // namespace can

// global C ISR handler called by HAL
extern "C" void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* /*hcan*/) {
  if (can::Stm32CanDriver::instance() != nullptr) {
    can::Stm32CanDriver::instance()->handleRxInterrupt();
  }
}
