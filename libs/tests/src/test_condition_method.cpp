#include <gtest/gtest.h>

import pludux;

using namespace pludux;

TEST(AnyConditionMethodTest, RunOneMethod)
{
  const auto equal_condition = EqualMethod{ValueMethod{1.0}, ValueMethod{1.0}};
  const auto condition_method = AnyConditionMethod{equal_condition};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto context = AnySeriesMethodContext{};

  const auto result = condition_method(asset_data, context);
  EXPECT_TRUE(result);

  const auto casted_condition = condition_method_cast<EqualMethod>(condition_method);
  ASSERT_NE(casted_condition, nullptr);
}

TEST(AnyConditionMethodTest, EqualityOperator)
{
  const auto condition1 = TrueMethod{};
  const auto condition2 = TrueMethod{};
  const auto condition_method1 = AnyConditionMethod{condition1};
  const auto condition_method2 = AnyConditionMethod{condition2};

  EXPECT_TRUE(condition_method1 == condition_method2);
  EXPECT_FALSE(condition_method1 != condition_method2);
  EXPECT_EQ(condition_method1, condition_method2);
}

TEST(AnyConditionMethodTest, NotEqualOperator)
{
  const auto condition1 = TrueMethod{};
  const auto condition2 = FalseMethod{};
  const auto condition_method1 = AnyConditionMethod{condition1};
  const auto condition_method2 = AnyConditionMethod{condition2};

  EXPECT_TRUE(condition_method1 != condition_method2);
  EXPECT_FALSE(condition_method1 == condition_method2);
}