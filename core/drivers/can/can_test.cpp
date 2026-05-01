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
