#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(BooleanMethodTest, TrueMethod)
{
  const auto filter = TrueMethod{};
  const auto asset_data = AssetHistory{};

  EXPECT_TRUE(filter(asset_data, context));
}

TEST(BooleanMethodTest, FalseMethod)
{
  const auto filter = FalseMethod{};
  const auto asset_data = AssetHistory{};

  EXPECT_FALSE(filter(asset_data, context));
}

TEST(BooleanMethodTest, TrueMethodEquality)
{
  const auto filter1 = TrueMethod{};
  const auto filter2 = TrueMethod{};

  EXPECT_TRUE(filter1 == filter2);
  EXPECT_FALSE(filter1 != filter2);
  EXPECT_EQ(filter1, filter2);
}