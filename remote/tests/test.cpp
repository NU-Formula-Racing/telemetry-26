#include "app.hpp"
#include "gtest/gtest.h"

TEST(AppTest, SampleTest) { EXPECT_EQ(1, 1); }

TEST(AppTest, DummyTest) {
  App app;
  EXPECT_EQ(app.run(), 5);
}