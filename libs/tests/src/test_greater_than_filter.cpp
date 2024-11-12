#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener/greater_than_filter.hpp>
#include <pludux/screener/value_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(GreaterThanFilterTest, TargetGreaterThanThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 40.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter = GreaterThanFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset = pludux::Asset("");

  EXPECT_TRUE(filter(asset));
}

TEST(GreaterThanFilterTest, TargetEqualToThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter = GreaterThanFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset = pludux::Asset("");

  EXPECT_FALSE(filter(asset));
}

TEST(GreaterThanFilterTest, TargetLessThanThreshold)
{
  const auto target_value = 30.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter = GreaterThanFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset = pludux::Asset("");

  EXPECT_FALSE(filter(asset));
}

