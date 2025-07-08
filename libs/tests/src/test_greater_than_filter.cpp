import pludux.asset_history;
import pludux.asset_snapshot;
import pludux.screener;
import pludux.series;

#include <gtest/gtest.h>
#include <pludux/screener.hpp>

using namespace pludux::screener;

TEST(GreaterThanFilterTest, TargetGreaterThanThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 40.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   GreaterThanFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_TRUE(filter(asset_data));
}

TEST(GreaterThanFilterTest, TargetEqualToThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   GreaterThanFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_FALSE(filter(asset_data));
}

TEST(GreaterThanFilterTest, TargetLessThanThreshold)
{
  const auto target_value = 30.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   GreaterThanFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_FALSE(filter(asset_data));
}

TEST(GreaterThanFilterTest, EqualityOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto greater_than_filter1 =
   GreaterThanFilter{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 50.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto greater_than_filter2 =
   GreaterThanFilter{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(greater_than_filter1 == greater_than_filter2);
  EXPECT_FALSE(greater_than_filter1 != greater_than_filter2);
  EXPECT_EQ(greater_than_filter1, greater_than_filter2);
}

TEST(GreaterThanFilterTest, NotEqualOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto greater_than_filter1 =
   GreaterThanFilter{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 60.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto greater_than_filter2 =
   GreaterThanFilter{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(greater_than_filter1 != greater_than_filter2);
  EXPECT_FALSE(greater_than_filter1 == greater_than_filter2);
  EXPECT_NE(greater_than_filter1, greater_than_filter2);
}