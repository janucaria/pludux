#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(GreaterThanMethodTest, TargetGreaterThanThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 40.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   GreaterThanMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(filter(asset_data, context));
}

TEST(GreaterThanMethodTest, TargetEqualToThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   GreaterThanMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(filter(asset_data, context));
}

TEST(GreaterThanMethodTest, TargetLessThanThreshold)
{
  const auto target_value = 30.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   GreaterThanMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(filter(asset_data, context));
}

TEST(GreaterThanMethodTest, EqualityOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto greater_than_filter1 =
   GreaterThanMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 50.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto greater_than_filter2 =
   GreaterThanMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(greater_than_filter1 == greater_than_filter2);
  EXPECT_FALSE(greater_than_filter1 != greater_than_filter2);
  EXPECT_EQ(greater_than_filter1, greater_than_filter2);
}

TEST(GreaterThanMethodTest, NotEqualOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto greater_than_filter1 =
   GreaterThanMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 60.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto greater_than_filter2 =
   GreaterThanMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(greater_than_filter1 != greater_than_filter2);
  EXPECT_FALSE(greater_than_filter1 == greater_than_filter2);
  EXPECT_NE(greater_than_filter1, greater_than_filter2);
}