#include <gtest/gtest.h>

#include <variant>

import pludux;

using namespace pludux;

TEST(ErasedSeriesMethodTest, RunOneMethod)
{
  const auto value_method = ValueMethod{1.0};
  const auto erased_series_method =
   ErasedSeriesMethod<ErasedSeriesMethodContext>{value_method};

  const auto casted_method =
   series_method_cast<ValueMethod>(erased_series_method);
  ASSERT_NE(casted_method, nullptr);

  const auto asset_data = AssetHistory{};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = ErasedSeriesMethodContext{};

  const auto result =
   evaluate_series_method(erased_series_method, asset_snapshot, context);

  EXPECT_DOUBLE_EQ(casted_method->value(), result);
}

TEST(ErasedSeriesMethodTest, UsesConcreteContextType)
{
  const auto method = ErasedSeriesMethod<std::monostate>{ValueMethod{1.0}};
  const auto asset_snapshot = AssetSnapshot{AssetHistory{}};

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(method, asset_snapshot, std::monostate{}), 1.0);
}

TEST(ErasedSeriesMethodTest, NestedErasedSeriesMethod)
{
  const auto value_method = ValueMethod{1.0};
  const auto inner_erased_series_method =
   ErasedSeriesMethod<ErasedSeriesMethodContext>{value_method};
  const auto outer_erased_series_method =
   ErasedSeriesMethod<ErasedSeriesMethodContext>{inner_erased_series_method};

  const auto casted_inner_method =
   series_method_cast<ValueMethod>(outer_erased_series_method);
  ASSERT_NE(casted_inner_method, nullptr);

  const auto asset_data = AssetHistory{};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = ErasedSeriesMethodContext{};

  const auto result =
   evaluate_series_method(outer_erased_series_method, asset_snapshot, context);

  EXPECT_DOUBLE_EQ(casted_inner_method->value(), result);
}

TEST(ErasedSeriesMethodTest, EqualityOperator)
{
  const auto erased_method1 =
   ErasedSeriesMethod<ErasedSeriesMethodContext>{ValueMethod{1.0}};
  const auto erased_method2 =
   ErasedSeriesMethod<ErasedSeriesMethodContext>{ValueMethod{1.0}};

  EXPECT_TRUE(erased_method1 == erased_method2);
  EXPECT_EQ(erased_method1, erased_method2);
}

TEST(ErasedSeriesMethodTest, NotEqualOperator)
{
  const auto erased_method1 =
   ErasedSeriesMethod<ErasedSeriesMethodContext>{DataMethod{"close"}};
  const auto erased_method2 =
   ErasedSeriesMethod<ErasedSeriesMethodContext>{DataMethod{"open"}};

  EXPECT_TRUE(erased_method1 != erased_method2);
  EXPECT_NE(erased_method1, erased_method2);
}
