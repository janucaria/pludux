#include <gtest/gtest.h>

#include "test_method_context.hpp"

import pludux;

using namespace pludux;

const auto context = StatelessMethodContext{};

TEST(GreaterThanMethodTest, TargetGreaterThanThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 40.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto condition =
   GreaterThanMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  EXPECT_TRUE(evaluate_series_method(condition, asset_snapshot[0], context));
}

TEST(GreaterThanMethodTest, TargetEqualToThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto condition =
   GreaterThanMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  EXPECT_FALSE(evaluate_series_method(condition, asset_snapshot[0], context));
}

TEST(GreaterThanMethodTest, TargetLessThanThreshold)
{
  const auto target_value = 30.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto condition =
   GreaterThanMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  EXPECT_FALSE(evaluate_series_method(condition, asset_snapshot[0], context));
}

TEST(GreaterThanMethodTest, EqualityOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto greater_than_condition1 =
   GreaterThanMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 50.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto greater_than_condition2 =
   GreaterThanMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(greater_than_condition1 == greater_than_condition2);
  EXPECT_FALSE(greater_than_condition1 != greater_than_condition2);
  EXPECT_EQ(greater_than_condition1, greater_than_condition2);
}

TEST(GreaterThanMethodTest, NotEqualOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto greater_than_condition1 =
   GreaterThanMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 60.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto greater_than_condition2 =
   GreaterThanMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(greater_than_condition1 != greater_than_condition2);
  EXPECT_FALSE(greater_than_condition1 == greater_than_condition2);
  EXPECT_NE(greater_than_condition1, greater_than_condition2);
}