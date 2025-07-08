import pludux.asset_history;
import pludux.asset_snapshot;
import pludux.screener;
import pludux.series;

#include <gtest/gtest.h>
#include <pludux/screener.hpp>

using namespace pludux::screener;

TEST(LessEqualFilterTest, TargetLessEqualThreshold)
{
  const auto target_value = 40.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   LessEqualFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_TRUE(filter(asset_data));
}

TEST(LessEqualFilterTest, TargetEqualToThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   LessEqualFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_TRUE(filter(asset_data));
}

TEST(LessEqualFilterTest, TargetGreaterThanThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 20.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter =
   LessEqualFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_FALSE(filter(asset_data));
}

TEST(LessEqualFilterTest, EqualityOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto less_equal_filter1 =
   LessEqualFilter{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 50.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto less_equal_filter2 =
   LessEqualFilter{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_TRUE(less_equal_filter1 == less_equal_filter2);
  EXPECT_FALSE(less_equal_filter1 != less_equal_filter2);
  EXPECT_EQ(less_equal_filter1, less_equal_filter2);
}

TEST(LessEqualFilterTest, NotEqualOperator)
{
  const auto target_value1 = 50.0;
  const auto threshold_value1 = 50.0;
  auto target_method1 = ValueMethod{target_value1};
  auto threshold_method1 = ValueMethod{threshold_value1};
  const auto less_equal_filter1 =
   LessEqualFilter{std::move(target_method1), std::move(threshold_method1)};

  const auto target_value2 = 60.0;
  const auto threshold_value2 = 50.0;
  auto target_method2 = ValueMethod{target_value2};
  auto threshold_method2 = ValueMethod{threshold_value2};
  const auto less_equal_filter2 =
   LessEqualFilter{std::move(target_method2), std::move(threshold_method2)};

  EXPECT_FALSE(less_equal_filter1 == less_equal_filter2);
  EXPECT_TRUE(less_equal_filter1 != less_equal_filter2);
  EXPECT_NE(less_equal_filter1, less_equal_filter2);
}
