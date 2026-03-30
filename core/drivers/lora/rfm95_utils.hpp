#pragma once

#include <cstdint>

namespace lora::rfm95 {

// RFM95 registers from datasheet: https://cdn.sparkfun.com/assets/a/9/6/1/0/RFM95W-V2.0.pdf
constexpr uint8_t REG_FIFO = 0x00;                  // FIFO read/write access
constexpr uint8_t REG_OP_MODE = 0x01;               // operating mode and LoRa selection
constexpr uint8_t REG_FRF_MSB = 0x06;               // RF carrier frequency (most significant bits)
constexpr uint8_t REG_FRF_MID = 0x07;               // RF carrier frequency (middle bits)
constexpr uint8_t REG_FRF_LSB = 0x08;               // RF carrier frequency (least significant bits)
constexpr uint8_t REG_PA_CONFIG = 0x09;             // PA selection and output power control
constexpr uint8_t REG_PA_RAMP = 0x0A;               // control of PA ramp time, low phase noise PLL
constexpr uint8_t REG_OCP = 0x0B;                   // overcurrent protection control
constexpr uint8_t REG_LNA = 0x0C;                   // LNA settings
constexpr uint8_t REG_FIFO_ADDR_PTR = 0x0D;         // FIFO SPI ptr
constexpr uint8_t REG_FIFO_TX_BASE_ADDR = 0x0E;     // start tx data
constexpr uint8_t REG_FIFO_RX_BASE_ADDR = 0x0F;     // start rx data
constexpr uint8_t REG_FIFO_RX_CURRENT_ADDR = 0x10;  // start address of last packet received
constexpr uint8_t REG_IRQ_FLAGS_MASK = 0x11;        // optional IRQ flag mask
constexpr uint8_t REG_IRQ_FLAGS = 0x12;             // IRQ flags
constexpr uint8_t REG_RX_NB_BYTES = 0x13;           // number of received bytes
constexpr uint8_t REG_RX_HEADER_CNT_VALUE_MSB = 0x14;  // number of valid headers received (MSB)
constexpr uint8_t REG_RX_HEADER_CNT_VALUE_LSB = 0x15;  // number of valid headers received (LSB)
constexpr uint8_t REG_RX_PACKET_CNT_VALUE_MSB = 0x16;  // number of packets received (MSB)
constexpr uint8_t REG_RX_PACKET_CNT_VALUE_LSB = 0x17;  // number of packets received (LSB)
constexpr uint8_t REG_MODEM_STAT = 0x18;               // live LoRa modem status
constexpr uint8_t REG_PKT_SNR_VALUE = 0x19;            // estimation of last packet SNR
constexpr uint8_t REG_PKT_RSSI_VALUE = 0x1A;           // RSSI of last packet
constexpr uint8_t REG_RSSI_VALUE = 0x1B;               // current RSSI
constexpr uint8_t REG_HOP_CHANNEL = 0x1C;              // FHSS start channel
constexpr uint8_t REG_MODEM_CONFIG_1 = 0x1D;           // modem PHY config 1
constexpr uint8_t REG_MODEM_CONFIG_2 = 0x1E;           // modem PHY config 2
constexpr uint8_t REG_SYMB_TIMEOUT_LSB = 0x1F;         // receiver timeout value
constexpr uint8_t REG_PREAMBLE_MSB = 0x20;             // size of preamble (MSB)
constexpr uint8_t REG_PREAMBLE_LSB = 0x21;             // size of preamble (LSB)
constexpr uint8_t REG_PAYLOAD_LENGTH = 0x22;           // LoRa payload length
constexpr uint8_t REG_MAX_PAYLOAD_LENGTH = 0x23;       // LoRa max payload length
constexpr uint8_t REG_HOP_PERIOD = 0x24;               // FHSS hop period
constexpr uint8_t REG_FIFO_RX_BYTE_ADDR = 0x25;        // address of last byte written in FIFO
constexpr uint8_t REG_MODEM_CONFIG_3 = 0x26;           // modem PHY config 3
constexpr uint8_t REG_FEI_MSB = 0x28;                  // estimated frequency error (MSB)
constexpr uint8_t REG_FEI_MID = 0x29;                  // estimated frequency error (middle bits)
constexpr uint8_t REG_FEI_LSB = 0x2A;        // estimated frequency error (least significant bits)
constexpr uint8_t REG_RSSI_WIDEBAND = 0x2C;  // wideband RSSI measurement
constexpr uint8_t REG_IF_FREQ_1 = 0x2F;  // intermediate frequency config 1 ("optimize receiver")
constexpr uint8_t REG_IF_FREQ_2 = 0x30;  // intermediate frequency config 2 ("optimize receiver")
constexpr uint8_t REG_DETECTION_OPTIMIZE = 0x31;   // LoRa detection optimize for SF6
constexpr uint8_t REG_INVERT_IQ = 0x33;            // invert LoRa I and Q signals
constexpr uint8_t REG_HIGH_BW_OPTIMIZE_1 = 0x36;   // sensitivity optimization for 500kHz bandwidth
constexpr uint8_t REG_DETECTION_THRESHOLD = 0x37;  // LoRa detection threshold for SF6
constexpr uint8_t REG_SYNC_WORD = 0x39;            // LoRa sync word config
constexpr uint8_t REG_HIGH_BW_OPTIMIZE_2 =
    0x3A;                                    // sensitivity optimization for for 500kHz bandwidth
constexpr uint8_t REG_INVERT_IQ_2 = 0x3B;    // optimize for inverted IQ
constexpr uint8_t REG_DIO_MAPPING_1 = 0x40;  // mapping of pins DIO0 to DIO3
constexpr uint8_t REG_DIO_MAPPING_2 = 0x41;  // mapping of pins DIO4 and DIO5, ClkOut frequency
constexpr uint8_t REG_VERSION = 0x42;        // semtech ID relating the silicon revision
constexpr uint8_t REG_TCXO = 0x4B;           // TCXO or XTAL input setting
constexpr uint8_t REG_PA_DAC = 0x4D;         // higher power settings of the PA
constexpr uint8_t REG_FORMER_TEMP = 0x5B;    // stored temperature during the former IQ calibration
constexpr uint8_t REG_AGC_REF = 0x61;        // adjustment of the AGC thresholds (REF)
constexpr uint8_t REG_AGC_THRESH_1 = 0x62;   // adjustment of the AGC thresholds (THRESH_1)
constexpr uint8_t REG_AGC_THRESH_2 = 0x63;   // adjustment of the AGC thresholds (THRESH_2)
constexpr uint8_t REG_AGC_THRESH_3 = 0x64;   // adjustment of the AGC thresholds (THRESH_3)
constexpr uint8_t REG_PLL = 0x70;            // control of the PLL bandwidth

// RFM95 OpModes (0x01 REG_OP_MODE)
constexpr uint8_t OPMODE_LONG_RANGE_MODE = 0x80;  // 0: FSK/OOK, 1: LoRa
constexpr uint8_t OPMODE_ACCESS_SHARED_REG =
    0x40;  // 0: access LoRa registers page 0x0D-0x3F, 1: access FSK registers page (in LoRa mode)
           // 0x0D-0x3F
constexpr uint8_t OPMODE_LOW_FREQ_MODE_ON =
    0x08;                               // 0: HF (868/915MHz) operation, 1: LF (433MHz) operation
constexpr uint8_t OPMODE_SLEEP = 0x00;  // sleep mode
constexpr uint8_t OPMODE_STDBY = 0x01;  // standby mode
constexpr uint8_t OPMODE_FSTX = 0x02;   // frequency synthesis transmit mode
constexpr uint8_t OPMODE_TX = 0x03;     // transmit mode
constexpr uint8_t OPMODE_FSRX = 0x04;   // frequency synthesis receive mode
constexpr uint8_t OPMODE_RXCONTINUOUS = 0x05;  // receive continuous mode
constexpr uint8_t OPMODE_RXSINGLE = 0x06;      // receive single mode
constexpr uint8_t OPMODE_CAD = 0x07;           // channel activity detection mode

}  // namespace lora::rfm95
