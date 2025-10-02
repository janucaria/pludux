#include <gtest/gtest.h>

import pludux;

using namespace pludux::screener;

TEST(AnyMethodTest, RunOneMethod)
{
  const auto value_method = ValueMethod{1.0};
  const auto any_method = AnyMethod{value_method};

  const auto casted_method =
   any_method_cast<ValueMethod>(any_method);
  ASSERT_NE(casted_method, nullptr);

  const auto asset_data = pludux::AssetHistory{};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = AnyMethodContext{};

  const auto result = any_method(asset_snapshot, context);

  EXPECT_DOUBLE_EQ(casted_method->value(), result);
}

TEST(AnyMethodTest, NestedAnyMethod)
{
  const auto value_method = ValueMethod{1.0};
  const auto inner_any_method = AnyMethod{value_method};
  const auto outer_any_method = AnyMethod{inner_any_method};

  const auto casted_inner_method =
   any_method_cast<ValueMethod>(outer_any_method);
  ASSERT_NE(casted_inner_method, nullptr);

  const auto asset_data = pludux::AssetHistory{};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = AnyMethodContext{};

  const auto result = outer_any_method(asset_snapshot, context);

  EXPECT_DOUBLE_EQ(casted_inner_method->value(), result);
}

TEST(AnyMethodTest, EqualityOperator)
{
  const auto any_method1 = AnyMethod{ValueMethod{1.0}};
  const auto any_method2 = AnyMethod{ValueMethod{1.0}};

  EXPECT_TRUE(any_method1 == any_method2);
  EXPECT_EQ(any_method1, any_method2);
}

TEST(AnyMethodTest, NotEqualOperator)
{
  const auto any_method1 = AnyMethod{DataMethod{"close"}};
  const auto any_method2 = AnyMethod{DataMethod{"open"}};

  EXPECT_TRUE(any_method1 != any_method2);
  EXPECT_NE(any_method1, any_method2);
}
