#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(LessEqualMethodTest, TargetLessEqualThreshold)
{
  const auto target_value = 40.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto condition =
   LessEqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(condition(asset_data, context));
}

TEST(LessEqualMethodTest, TargetEqualToThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto condition =
   LessEqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(condition(asset_data, context));
}

TEST(LessEqualMethodTest, TargetGreaterThanThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 20.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto condition =
   LessEqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(condition(asset_data, context));
}

TEST(LessEqualMethodTest, EqualityOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto less_equal_condition1 =
   LessEqualMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 50.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto less_equal_condition2 =
   LessEqualMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(less_equal_condition1 == less_equal_condition2);
  EXPECT_FALSE(less_equal_condition1 != less_equal_condition2);
  EXPECT_EQ(less_equal_condition1, less_equal_condition2);
}

TEST(LessEqualMethodTest, NotEqualOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto less_equal_condition1 =
   LessEqualMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 60.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto less_equal_condition2 =
   LessEqualMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_FALSE(less_equal_condition1 == less_equal_condition2);
  EXPECT_TRUE(less_equal_condition1 != less_equal_condition2);
  EXPECT_NE(less_equal_condition1, less_equal_condition2);
}
