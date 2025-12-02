#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

const auto context = std::monostate{};

TEST(LookbackMethodTest, ConstructorInitialization)
{
  auto lookback = LookbackMethod{CloseMethod{}, 1};

  EXPECT_EQ(lookback.period(), 1);
  EXPECT_EQ(lookback.source(), CloseMethod{});

  lookback.period(2);
  EXPECT_EQ(lookback.period(), 2);

  auto nested_lookback = LookbackMethod{lookback, 3};
  EXPECT_EQ(nested_lookback.period(), 5);
  EXPECT_EQ(nested_lookback.source(), CloseMethod{});
}

TEST(LookbackMethodTest, RunAllMethodClose)
{
  const auto asset_data = AssetHistory{{"Close", {4.0, 4.1, 4.2}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto lookback_method = LookbackMethod{CloseMethod{}, 1};

  EXPECT_EQ((lookback_method(asset_snapshot[0], context)), 4.1);
  EXPECT_EQ((lookback_method(asset_snapshot[1], context)), 4.2);
  EXPECT_TRUE(std::isnan((lookback_method(asset_snapshot[2], context))));
}

TEST(LookbackMethodTest, EqualityComparisonOperator)
{
  const auto lookback_method1 = LookbackMethod{CloseMethod{}, 1};
  const auto lookback_method2 = LookbackMethod{CloseMethod{}, 1};
  const auto lookback_method3 = LookbackMethod{CloseMethod{}, 2};

  EXPECT_EQ(lookback_method1, lookback_method2);
  EXPECT_NE(lookback_method1, lookback_method3);
  EXPECT_NE(lookback_method2, lookback_method3);
}