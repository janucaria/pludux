#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(BooleanMethodTest, TrueMethod)
{
  const auto condition = TrueMethod{};
  const auto asset_data = AssetHistory{};

  EXPECT_TRUE(condition(asset_data, context));
}

TEST(BooleanMethodTest, FalseMethod)
{
  const auto condition = FalseMethod{};
  const auto asset_data = AssetHistory{};

  EXPECT_FALSE(condition(asset_data, context));
}

TEST(BooleanMethodTest, TrueMethodEquality)
{
  const auto condition1 = TrueMethod{};
  const auto condition2 = TrueMethod{};

  EXPECT_TRUE(condition1 == condition2);
  EXPECT_FALSE(condition1 != condition2);
  EXPECT_EQ(condition1, condition2);
}