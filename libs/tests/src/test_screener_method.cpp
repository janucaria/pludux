#include <gtest/gtest.h>

import pludux;

using namespace pludux::screener;

TEST(ScreenerMethodTest, RunOneMethod)
{
  const auto value_method = ValueMethod{1.0};
  const auto screener_method = ScreenerMethod{value_method};

  const auto casted_method =
   screener_method_cast<ValueMethod<>>(screener_method);
  ASSERT_NE(casted_method, nullptr);

  const auto asset_data = pludux::AssetHistory{};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = AnyMethodContext{};

  const auto result = screener_method(asset_snapshot, context);

  EXPECT_DOUBLE_EQ(casted_method->value(), result);
}

TEST(ScreenerMethodTest, NestedScreenerMethod)
{
  const auto value_method = ValueMethod{1.0};
  const auto inner_screener_method = ScreenerMethod{value_method};
  const auto outer_screener_method = ScreenerMethod{inner_screener_method};

  const auto casted_inner_method =
   screener_method_cast<ValueMethod<>>(outer_screener_method);
  ASSERT_NE(casted_inner_method, nullptr);

  const auto asset_data = pludux::AssetHistory{};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = AnyMethodContext{};

  const auto result = outer_screener_method(asset_snapshot, context);

  EXPECT_DOUBLE_EQ(casted_inner_method->value(), result);
}

TEST(ScreenerMethodTest, EqualityOperator)
{
  const auto screener_method1 = ScreenerMethod{ValueMethod{1.0}};
  const auto screener_method2 = ScreenerMethod{ValueMethod{1.0}};

  EXPECT_TRUE(screener_method1 == screener_method2);
  EXPECT_EQ(screener_method1, screener_method2);
}

TEST(ScreenerMethodTest, NotEqualOperator)
{
  const auto screener_method1 = ScreenerMethod{DataMethod{"close"}};
  const auto screener_method2 = ScreenerMethod{DataMethod{"open"}};

  EXPECT_TRUE(screener_method1 != screener_method2);
  EXPECT_NE(screener_method1, screener_method2);
}
