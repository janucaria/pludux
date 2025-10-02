#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(NotEqualMethodTest, TargetEqualThreshold)
{
  const auto target_value = 40.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   NotEqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(filter(asset_data, context));
}

TEST(NotEqualMethodTest, TargetEqualToThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   NotEqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(filter(asset_data, context));
}

TEST(NotEqualMethodTest, TargetGreaterThanThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 20.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   NotEqualMethod{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(filter(asset_data, context));
}

TEST(NotEqualMethodTest, EqualityOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto not_equal_filter1 =
   NotEqualMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 50.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto not_equal_filter2 =
   NotEqualMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(not_equal_filter1 == not_equal_filter2);
  EXPECT_FALSE(not_equal_filter1 != not_equal_filter2);
  EXPECT_EQ(not_equal_filter1, not_equal_filter2);
}

TEST(NotEqualMethodTest, NotEqualOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto not_equal_filter1 =
   NotEqualMethod{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 60.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto not_equal_filter2 =
   NotEqualMethod{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(not_equal_filter1 != not_equal_filter2);
  EXPECT_FALSE(not_equal_filter1 == not_equal_filter2);
  EXPECT_NE(not_equal_filter1, not_equal_filter2);
}
