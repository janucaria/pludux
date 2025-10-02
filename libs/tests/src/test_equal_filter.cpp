#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(EqualMethodTest, TargetEqualThreshold)
{
  const auto target_value = 40.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   EqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(filter(asset_data, context));
}

TEST(EqualMethodTest, TargetEqualToThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   EqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(filter(asset_data, context));
}

TEST(EqualMethodTest, TargetGreaterThanThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 20.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   EqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(filter(asset_data, context));
}

TEST(EqualMethodTest, EqualityOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto equal_filter1 =
   EqualMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 50.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto equal_filter2 =
   EqualMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(equal_filter1 == equal_filter2);
  EXPECT_FALSE(equal_filter1 != equal_filter2);
  EXPECT_EQ(equal_filter1, equal_filter2);
}

TEST(EqualMethodTest, NotEqualOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto equal_filter1 =
   EqualMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 60.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto equal_filter2 =
   EqualMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(equal_filter1 != equal_filter2);
  EXPECT_FALSE(equal_filter1 == equal_filter2);
  EXPECT_NE(equal_filter1, equal_filter2);
}

