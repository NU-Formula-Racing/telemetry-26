#include "odometer.hpp"

#include <gtest/gtest.h>

#include <cstdint>

class OdometerTestFixture : public ::testing::Test {
 protected:
  void SetUp() override { odometer_.init(0.0F); }

  odometer::Odometer& odometer() { return odometer_; }

 private:
  odometer::Odometer odometer_;
};

TEST_F(OdometerTestFixture, InitialMilesTest) {
  EXPECT_FLOAT_EQ(odometer().getMilesDriven(), 0.0F);
}

TEST_F(OdometerTestFixture, CalculateDistance1s) {
  odometer().updateMilesDriven(1000, 1000);

  EXPECT_GT(odometer().getMilesDriven(), 0.0F);
}

TEST_F(OdometerTestFixture, ConstantRpm1Milliseconds1) {
  EXPECT_FLOAT_EQ(odometer().getMilesDriven(), 0.0F);

  odometer().updateMilesDriven(1, 1);
  EXPECT_FLOAT_EQ(odometer().getMilesDriven(), 1.9444398973E-9F);
  odometer().updateMilesDriven(1, 2);
  EXPECT_FLOAT_EQ(odometer().getMilesDriven(), 5.8333196918E-9F);
}

TEST_F(OdometerTestFixture, ZeroRpmMilliseconds1) {
  EXPECT_FLOAT_EQ(odometer().getMilesDriven(), 0.0F);

  odometer().updateMilesDriven(100, 1000);
  EXPECT_FLOAT_EQ(odometer().getMilesDriven(), 1.9444398973E-4F);
  odometer().updateMilesDriven(0, 1001);
  EXPECT_NEAR(odometer().getMilesDriven(), 1.9444398973E-4F, 1E-6F);
}

TEST_F(OdometerTestFixture, ReversingDoesntDecreaseMiles) {
  odometer().init(100.0F);
  EXPECT_FLOAT_EQ(odometer().getMilesDriven(), 100.0F);

  odometer().updateMilesDriven(-500, 0);
  odometer().updateMilesDriven(-500, 1000);

  EXPECT_FLOAT_EQ(odometer().getMilesDriven(), 100.0F);
}

TEST_F(OdometerTestFixture, HandleTimerRolloverGracefully) {
  odometer().init(0.0F);

  uint32_t maxTime = std::numeric_limits<uint32_t>::max();

  // drive 0 RPM at t=maxTime - 10ms
  odometer().updateMilesDriven(0, maxTime - 10);
  // drive 1000 RPM at t=maxTime
  odometer().updateMilesDriven(1000, maxTime);
  float milesBeforeRollover = odometer().getMilesDriven();
  // drive 1000 RPM at t=9ms (after rollover)
  uint32_t newTime = 9;
  odometer().updateMilesDriven(1000, newTime);
  float milesAfterRollover = odometer().getMilesDriven();
  // calculate the distance added strictly during that 10ms window
  float milesAddedDuringRollover = milesAfterRollover - milesBeforeRollover;

  EXPECT_NEAR(milesAddedDuringRollover, 0.0000388F, 1E-6F);
}

TEST_F(OdometerTestFixture, InitWithExistingMiles) {
  odometer().init(500.5F);
  EXPECT_FLOAT_EQ(odometer().getMilesDriven(), 500.5F);

  // some driving
  odometer().updateMilesDriven(100, 0);
  odometer().updateMilesDriven(100, 1000);

  EXPECT_GT(odometer().getMilesDriven(), 500.5F);
}

TEST_F(OdometerTestFixture, CumulativeDriving) {
  odometer().init(0.0F);

  odometer().updateMilesDriven(1000, 0);
  odometer().updateMilesDriven(1000, 1000);
  float distance1 = odometer().getMilesDriven();

  odometer().updateMilesDriven(1000, 2000);
  float distance2 = odometer().getMilesDriven();

  // driving for 2 seconds at the same speed should yield exactly double the distance of 1 second
  EXPECT_FLOAT_EQ(distance2, distance1 * 2.0F);
}

TEST_F(OdometerTestFixture, MaxRpmSanityCheck) {
  odometer().init(0.0F);

  // drive at 30k RPM (absurdly high) for 10 seconds
  odometer().updateMilesDriven(30000, 0);
  odometer().updateMilesDriven(30000, 10000);

  float miles = odometer().getMilesDriven();

  // verify it doesn't result in Infinity or NaN
  EXPECT_FALSE(std::isnan(miles));
  EXPECT_FALSE(std::isinf(miles));
  EXPECT_GT(miles, 0.0F);
}

TEST(IntegratorTest, ZeroVelocity1sInterval) {
  odometer::Integrator integrator;
  integrator.setSum(0.0F);

  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(0.0F, 0), 0.0F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(0.0F, 1000), 0.0F);
}

TEST(IntegratorTest, ConstantVelocity10Over1sInterval) {
  // rectangle shape
  odometer::Integrator integrator;
  integrator.setSum(0.0F);

  // new_distance = 0s * (10 + 0)/2 = 0
  // total_distance = 0 + new_distance = 0
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(10.0F, 0), 0.0F);
  // same speed as before
  // new_distance = 1s * (10 + 10)/2 = 10
  // total_distance = 0 + new_distance = 10
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(10.0F, 1000), 10.0F);  // area = 10 * 1s = 10
  // same speed as before
  // new_distance = 1s * (10 + 10)/2 = 10
  // total_distance = 10 + new_distance = 20
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(10.0F, 2000), 20.0F);
}

TEST(IntegratorTest, ConstantVelocity1Over1msInterval) {
  odometer::Integrator integrator;
  integrator.setSum(0.0F);

  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(1.0F, 1), 0.0005F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(1.0F, 2), 0.0015F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(1.0F, 3), 0.0025F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(1.0F, 4), 0.0035F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(1.0F, 5), 0.0045F);
}

TEST(IntegratorTest, IncreasingVelocityOver1sInterval) {
  odometer::Integrator integrator;
  integrator.setSum(0.0F);

  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(1.0F, 1000), 0.5F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(2.0F, 2000), 2.0F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(10.0F, 3000), 8.0F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(100.0F, 3500), 35.5F);
}

TEST(IntegratorTest, DecreasingVelocityOver1sInterval) {
  odometer::Integrator integrator;
  integrator.setSum(0.0F);

  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(10.0F, 1000), 5.0F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(5.0F, 2000), 12.5F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(2.0F, 3000), 16.0F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(1.0F, 4000), 17.5F);
}

TEST(IntegratorTest, IncreasingDecreasingVelocityOver1sInterval) {
  odometer::Integrator integrator;
  integrator.setSum(0.0F);

  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(1.0F, 1000), 0.5F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(10.0F, 2000), 6.0F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(5.0F, 3000), 13.5F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(2.0F, 4000), 17.0F);
}

TEST(IntegratorTest, NegativeVelocityYieldsNegativeArea) {
  odometer::Integrator integrator;
  integrator.setSum(0.0F);

  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(-10.0F, 0), 0.0F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(-10.0F, 1000), -10.0F);
}

TEST(IntegratorTest, ZeroTimeDeltaYieldsZeroArea) {
  odometer::Integrator integrator;
  integrator.setSum(0.0F);

  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(100.0F, 0), 0.0F);
  EXPECT_FLOAT_EQ(integrator.trapezoidIntegrate(100.0F, 0), 0.0F);
}