#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

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
