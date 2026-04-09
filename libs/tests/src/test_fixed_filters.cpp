#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(FixedMethodTest, AlwaysMethod)
{
  const auto condition = AlwaysMethod{};
  const auto asset_data = AssetHistory{};

  EXPECT_TRUE(condition(asset_data, context));
}

TEST(FixedMethodTest, NeverMethod)
{
  const auto condition = NeverMethod{};
  const auto asset_data = AssetHistory{};

  EXPECT_FALSE(condition(asset_data, context));
}

TEST(FixedMethodTest, AlwaysMethodEquality)
{
  const auto condition1 = AlwaysMethod{};
  const auto condition2 = AlwaysMethod{};

  EXPECT_TRUE(condition1 == condition2);
  EXPECT_FALSE(condition1 != condition2);
  EXPECT_EQ(condition1, condition2);
}

TEST(FixedMethodTest, NeverMethodEquality)
{
  const auto condition1 = NeverMethod{};
  const auto condition2 = NeverMethod{};

  EXPECT_TRUE(condition1 == condition2);
  EXPECT_FALSE(condition1 != condition2);
  EXPECT_EQ(condition1, condition2);
}