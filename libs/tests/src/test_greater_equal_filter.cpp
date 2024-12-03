#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(GreaterEqualFilterTest, TargetGreaterEqualThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 40.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter = GreaterEqualFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset = pludux::Asset{""};
  const auto asset_data = AssetDataProvider{asset};

  EXPECT_TRUE(filter(asset_data));
}

TEST(GreaterEqualFilterTest, TargetEqualToThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter = GreaterEqualFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset = pludux::Asset{""};
  const auto asset_data = AssetDataProvider{asset};

  EXPECT_TRUE(filter(asset_data));
}

TEST(GreaterEqualFilterTest, TargetLessThanThreshold)
{
  const auto target_value = 30.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter = GreaterEqualFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset = pludux::Asset{""};
  const auto asset_data = AssetDataProvider{asset};

  EXPECT_FALSE(filter(asset_data));
}

