#include <gtest/gtest.h>

#include "test_method_context.hpp"

import pludux;

using namespace pludux;

using SignalAlwaysMethod = pludux::TrueMethod;
using SignalNeverMethod = pludux::FalseMethod;
using SignalEqualMethod =
 pludux::EqualMethod<ErasedSeriesMethod<StatelessMethodContext>,
                     ErasedSeriesMethod<StatelessMethodContext>>;

TEST(AnyConditionMethodTest, RunOneMethod)
{
  const auto equal_condition =
   SignalEqualMethod{ValueMethod{1.0}, ValueMethod{1.0}};
  const auto condition_method =
   ErasedSeriesMethod<StatelessMethodContext>{equal_condition};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = StatelessMethodContext{};

  const auto result =
   evaluate_series_method(condition_method, asset_snapshot[0], context);
  EXPECT_TRUE(result);

  const auto casted_condition =
   series_method_cast<SignalEqualMethod>(condition_method);
  ASSERT_NE(casted_condition, nullptr);
}

TEST(AnyConditionMethodTest, EqualityOperator)
{
  const auto condition1 = SignalAlwaysMethod{};
  const auto condition2 = SignalAlwaysMethod{};
  const auto condition_method1 =
   ErasedSeriesMethod<StatelessMethodContext>{condition1};
  const auto condition_method2 =
   ErasedSeriesMethod<StatelessMethodContext>{condition2};

  EXPECT_TRUE(condition_method1 == condition_method2);
  EXPECT_FALSE(condition_method1 != condition_method2);
  EXPECT_EQ(condition_method1, condition_method2);
}

TEST(AnyConditionMethodTest, NotEqualOperator)
{
  const auto condition1 = SignalAlwaysMethod{};
  const auto condition2 = SignalNeverMethod{};
  const auto condition_method1 =
   ErasedSeriesMethod<StatelessMethodContext>{condition1};
  const auto condition_method2 =
   ErasedSeriesMethod<StatelessMethodContext>{condition2};

  EXPECT_TRUE(condition_method1 != condition_method2);
  EXPECT_FALSE(condition_method1 == condition_method2);
}

TEST(AnyConditionMethodTest, SignalMethodsAreCoercedToNumericSeriesResults)
{
  const auto asset_data = AssetHistory{{"close", {1.0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = StatelessMethodContext{};

  const auto always =
   ErasedSeriesMethod<StatelessMethodContext>{SignalAlwaysMethod{}};
  const auto never =
   ErasedSeriesMethod<StatelessMethodContext>{SignalNeverMethod{}};
  const auto and_condition = ErasedSeriesMethod<StatelessMethodContext>{
   LogicalAndMethod{SignalAlwaysMethod{}, SignalNeverMethod{}}};
  const auto or_condition = ErasedSeriesMethod<StatelessMethodContext>{
   LogicalOrMethod{SignalAlwaysMethod{}, SignalNeverMethod{}}};

  EXPECT_DOUBLE_EQ(evaluate_series_method(always, asset_snapshot[0], context),
                   1.0);
  EXPECT_DOUBLE_EQ(evaluate_series_method(never, asset_snapshot[0], context),
                   0.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(and_condition, asset_snapshot[0], context), 0.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(or_condition, asset_snapshot[0], context), 1.0);
}
