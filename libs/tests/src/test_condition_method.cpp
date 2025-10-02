#include <gtest/gtest.h>

import pludux;

using namespace pludux;

TEST(AnyConditionMethodTest, RunOneMethod)
{
  const auto equal_filter = EqualMethod{ValueMethod{1.0}, ValueMethod{1.0}};
  const auto condition_method = AnyConditionMethod{equal_filter};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto context = AnySeriesMethodContext{};

  const auto result = condition_method(asset_data, context);
  EXPECT_TRUE(result);

  const auto casted_filter = condition_method_cast<EqualMethod>(condition_method);
  ASSERT_NE(casted_filter, nullptr);
}

TEST(AnyConditionMethodTest, EqualityOperator)
{
  const auto filter1 = TrueMethod{};
  const auto filter2 = TrueMethod{};
  const auto condition_method1 = AnyConditionMethod{filter1};
  const auto condition_method2 = AnyConditionMethod{filter2};

  EXPECT_TRUE(condition_method1 == condition_method2);
  EXPECT_FALSE(condition_method1 != condition_method2);
  EXPECT_EQ(condition_method1, condition_method2);
}

TEST(AnyConditionMethodTest, NotEqualOperator)
{
  const auto filter1 = TrueMethod{};
  const auto filter2 = FalseMethod{};
  const auto condition_method1 = AnyConditionMethod{filter1};
  const auto condition_method2 = AnyConditionMethod{filter2};

  EXPECT_TRUE(condition_method1 != condition_method2);
  EXPECT_FALSE(condition_method1 == condition_method2);
}