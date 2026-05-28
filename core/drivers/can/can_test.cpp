#include <gtest/gtest.h>

#include "can_types.hpp"

namespace {

// arbitrary dummy message for testing
#pragma pack(push, 1)
struct DummyTestMsg {
  static constexpr uint32_t ID = 0x123;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  uint16_t temperature;
  uint8_t status;
};
#pragma pack(pop)

//  message with max payload size and CAN ID
#pragma pack(push, 1)
struct MaxSizeTestMsg {
  static constexpr uint32_t ID = 0x1FFFFFFF;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::EXTENDED;

  uint32_t data1;
  uint32_t data2;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct OneByteMsg {
  static constexpr uint32_t ID = 0x111;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;
  uint8_t value;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct FourByteMsg {
  static constexpr uint32_t ID = 0x222;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;
  uint32_t value;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MixedTypesMsg {
  static constexpr uint32_t ID = 0x333;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;
  uint8_t a;
  uint32_t b;
  uint16_t c;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SignalTestMsg {
  static constexpr uint32_t ID = 0x444;
  static constexpr can::CanIdType ID_TYPE = can::CanIdType::STANDARD;

  // Factor: 0.1, Offset: 0.0 (e.g. 12.5 -> 125)
  can::CanSignal<uint32_t, float, 0.1f, 0.0f> odometer;
  // Factor: 1.0, Offset: -50.0 (e.g. 20.0 -> 70)
  can::CanSignal<uint8_t, float, 1.0f, -50.0f> temperature;
  // Factor: 1.0, Offset: 0.0, raw: uint8_t, physical: uint8_t
  can::CanSignal<uint8_t, uint8_t, 1.0f, 0.0f> status;
};
#pragma pack(pop)

}  // namespace

TEST(CanEncodeDecodeTest, EncodeSetsHeaderAndDataCorrectly) {
  DummyTestMsg msg{};
  msg.temperature = 0xABCD;
  msg.status = 0x01;

  can::CanFrame frame = can::encode(msg);

  EXPECT_EQ(frame.id, DummyTestMsg::ID);
  EXPECT_EQ(frame.idType, DummyTestMsg::ID_TYPE);
  EXPECT_EQ(frame.dlc, sizeof(DummyTestMsg));
}

TEST(CanEncodeDecodeTest, DecodeReconstructsMessage) {
  DummyTestMsg original{};
  original.temperature = 0x1234;
  original.status = 0x55;

  can::CanFrame frame = can::encode(original);
  auto decoded = can::decode<DummyTestMsg>(frame);

  EXPECT_EQ(decoded.temperature, original.temperature);
  EXPECT_EQ(decoded.status, original.status);
}

TEST(CanEncodeDecodeTest, MaxSizeMessageRoundTrip) {
  MaxSizeTestMsg original{};
  original.data1 = 0xDEADBEEF;
  original.data2 = 0xCAFECAFE;

  can::CanFrame frame = can::encode(original);
  EXPECT_EQ(frame.dlc, 8);

  auto decoded = can::decode<MaxSizeTestMsg>(frame);
  EXPECT_EQ(decoded.data1, original.data1);
  EXPECT_EQ(decoded.data2, original.data2);
}

TEST(CanFrameTest, DataToStringFormatsCorrectly) {
  can::CanFrame frame{};
  frame.data = {0x0A, 0x1B, 0x2C, 0x3D, 0x4E, 0x5F, 0x99, 0xFF};

  std::string expected = "0A 1B 2C 3D 4E 5F 99 FF";
  EXPECT_EQ(frame.dataToString(), expected);
}

TEST(CanFrameTest, DataToStringHandlesZeros) {
  can::CanFrame frame{};
  frame.data = {0, 0, 0, 0, 0, 0, 0, 0};

  std::string expected = "00 00 00 00 00 00 00 00";
  EXPECT_EQ(frame.dataToString(), expected);
}

TEST(CanEncodeDecodeTest, EncodeDecodeSmallMessage) {
  OneByteMsg msg{};
  msg.value = 0x42;

  can::CanFrame frame = can::encode(msg);
  EXPECT_EQ(frame.dlc, 1);
  EXPECT_EQ(frame.data.at(0), 0x42);

  auto decoded = can::decode<OneByteMsg>(frame);
  EXPECT_EQ(decoded.value, 0x42);
}

TEST(CanEncodeDecodeTest, DecodePreventsBufferOverrunOnMismatchedDlc) {
  can::CanFrame frame{};
  frame.dlc = 2;                                      //  pretend we only received 2 bytes
  frame.data = {0x11, 0x22, 0x33, 0x44, 0, 0, 0, 0};  // data has 4 bytes of data though

  auto decoded = can::decode<FourByteMsg>(frame);

  std::array<uint8_t, 4> rawBytes{};
  std::memcpy(rawBytes.data(), &decoded.value, sizeof(rawBytes));
  EXPECT_EQ(rawBytes.at(0), 0x11);
  EXPECT_EQ(rawBytes.at(1), 0x22);
  EXPECT_EQ(rawBytes.at(2), 0x00);
  EXPECT_EQ(rawBytes.at(3), 0x00);
}

TEST(CanEncodeDecodeTest, StructIsProperlyPacked) {
  // 1 byte (uint8_t) + 4 bytes (uint32_t) + 2 bytes (uint16_t) = 7 bytes
  EXPECT_EQ(sizeof(MixedTypesMsg), 7);
}

// --- New Tests for CanSignal ---

TEST(CanSignalTest, FromPhysicalAppliesFactor) {
  can::CanSignal<uint32_t, float, 0.1f, 0.0f> odometer;
  odometer.fromPhysical(12.5f);
  // (12.5 - 0.0) / 0.1 = 125
  EXPECT_EQ(odometer.rawValue, 125);
}

TEST(CanSignalTest, FromPhysicalAppliesOffset) {
  can::CanSignal<uint8_t, float, 1.0f, -50.0f> temperature;
  temperature.fromPhysical(20.0f);
  // (20.0 - (-50.0)) / 1.0 = 70
  EXPECT_EQ(temperature.rawValue, 70);
}

TEST(CanSignalTest, ToPhysicalReversesFactor) {
  can::CanSignal<uint32_t, float, 0.1f, 0.0f> odometer;
  odometer.rawValue = 125;
  // (125 * 0.1) + 0.0 = 12.5
  EXPECT_FLOAT_EQ(odometer.toPhysical(), 12.5f);
}

TEST(CanSignalTest, ToPhysicalReversesOffset) {
  can::CanSignal<uint8_t, float, 1.0f, -50.0f> temperature;
  temperature.rawValue = 70;
  // (70 * 1.0) + (-50.0) = 20.0
  EXPECT_FLOAT_EQ(temperature.toPhysical(), 20.0f);
}

TEST(CanSignalTest, UnscaledIntegerTypesWork) {
  can::CanSignal<uint8_t, uint8_t, 1.0f, 0.0f> status;
  status.fromPhysical(static_cast<uint8_t>(42));
  EXPECT_EQ(status.rawValue, 42);
  EXPECT_EQ(status.toPhysical(), 42);
}

TEST(CanSignalTest, ConstructorCallsFromPhysical) {
  can::CanSignal<uint32_t, float, 0.1f, 0.0f> odometer(15.5f);
  EXPECT_EQ(odometer.rawValue, 155);
}

TEST(CanSignalTest, EncodeDecodeWithCanSignal) {
  SignalTestMsg original{};
  original.odometer.fromPhysical(100.5f);                    // Raw: 1005
  original.temperature.fromPhysical(25.0f);                  // Raw: 75
  original.status.fromPhysical(static_cast<uint8_t>(0xAA));  // Raw: 0xAA

  can::CanFrame frame = can::encode(original);
  EXPECT_EQ(frame.dlc, sizeof(SignalTestMsg));

  auto decoded = can::decode<SignalTestMsg>(frame);

  EXPECT_FLOAT_EQ(decoded.odometer.toPhysical(), 100.5f);
  EXPECT_FLOAT_EQ(decoded.temperature.toPhysical(), 25.0f);
  EXPECT_EQ(decoded.status.toPhysical(), 0xAA);
}