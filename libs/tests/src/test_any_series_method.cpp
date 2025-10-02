#include <gtest/gtest.h>

import pludux;

using namespace pludux;

TEST(AnySeriesMethodTest, RunOneMethod)
{
  const auto value_method = ValueMethod{1.0};
  const auto any_series_method = AnySeriesMethod{value_method};

  const auto casted_method =
   series_method_cast<ValueMethod>(any_series_method);
  ASSERT_NE(casted_method, nullptr);

  const auto asset_data = AssetHistory{};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = AnySeriesMethodContext{};

  const auto result = any_series_method(asset_snapshot, context);

  EXPECT_DOUBLE_EQ(casted_method->value(), result);
}

TEST(AnySeriesMethodTest, NestedAnySeriesMethod)
{
  const auto value_method = ValueMethod{1.0};
  const auto inner_any_series_method = AnySeriesMethod{value_method};
  const auto outer_any_series_method = AnySeriesMethod{inner_any_series_method};

  const auto casted_inner_method =
   series_method_cast<ValueMethod>(outer_any_series_method);
  ASSERT_NE(casted_inner_method, nullptr);

  const auto asset_data = AssetHistory{};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = AnySeriesMethodContext{};

  const auto result = outer_any_series_method(asset_snapshot, context);

  EXPECT_DOUBLE_EQ(casted_inner_method->value(), result);
}

TEST(AnySeriesMethodTest, EqualityOperator)
{
  const auto any_method1 = AnySeriesMethod{ValueMethod{1.0}};
  const auto any_method2 = AnySeriesMethod{ValueMethod{1.0}};

  EXPECT_TRUE(any_method1 == any_method2);
  EXPECT_EQ(any_method1, any_method2);
}

TEST(AnySeriesMethodTest, NotEqualOperator)
{
  const auto any_method1 = AnySeriesMethod{DataMethod{"close"}};
  const auto any_method2 = AnySeriesMethod{DataMethod{"open"}};

  EXPECT_TRUE(any_method1 != any_method2);
  EXPECT_NE(any_method1, any_method2);
}
