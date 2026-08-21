#include <gtest/gtest.h>

#include "test_method_context.hpp"

import pludux;

using namespace pludux;

using SignalAlwaysMethod = pludux::TrueMethod;
using SignalNeverMethod = pludux::FalseMethod;

const auto context = StatelessMethodContext{};

TEST(FixedMethodTest, AlwaysMethod)
{
  const auto condition = SignalAlwaysMethod{};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  EXPECT_TRUE(evaluate_series_method(condition, asset_snapshot[0], context));
}

TEST(FixedMethodTest, NeverMethod)
{
  const auto condition = SignalNeverMethod{};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  EXPECT_FALSE(evaluate_series_method(condition, asset_snapshot[0], context));
}

TEST(FixedMethodTest, AlwaysMethodEquality)
{
  const auto condition1 = SignalAlwaysMethod{};
  const auto condition2 = SignalAlwaysMethod{};

  EXPECT_TRUE(condition1 == condition2);
  EXPECT_FALSE(condition1 != condition2);
  EXPECT_EQ(condition1, condition2);
}

TEST(FixedMethodTest, NeverMethodEquality)
{
  const auto condition1 = SignalNeverMethod{};
  const auto condition2 = SignalNeverMethod{};

  EXPECT_TRUE(condition1 == condition2);
  EXPECT_FALSE(condition1 != condition2);
  EXPECT_EQ(condition1, condition2);
}