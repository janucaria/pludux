#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(GreaterEqualMethodTest, TargetGreaterEqualThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 40.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto condition =
   GreaterEqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(condition(asset_data, context));
}

TEST(GreaterEqualMethodTest, TargetEqualToThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto condition =
   GreaterEqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(condition(asset_data, context));
}

TEST(GreaterEqualMethodTest, TargetLessThanThreshold)
{
  const auto target_value = 30.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto condition =
   GreaterEqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(condition(asset_data, context));
}

TEST(GreaterEqualMethodTest, EqualityOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto greater_equal_condition1 =
   GreaterEqualMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 50.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto greater_equal_condition2 =
   GreaterEqualMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(greater_equal_condition1 == greater_equal_condition2);
  EXPECT_FALSE(greater_equal_condition1 != greater_equal_condition2);
  EXPECT_EQ(greater_equal_condition1, greater_equal_condition2);
}

TEST(GreaterEqualMethodTest, NotEqualOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto greater_equal_condition1 =
   GreaterEqualMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 30.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto greater_equal_condition2 =
   GreaterEqualMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(greater_equal_condition1 != greater_equal_condition2);
  EXPECT_FALSE(greater_equal_condition1 == greater_equal_condition2);
  EXPECT_NE(greater_equal_condition1, greater_equal_condition2);
}

