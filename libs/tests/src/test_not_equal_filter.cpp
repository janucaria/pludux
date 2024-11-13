#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(NotEqualFilterTest, TargetEqualThreshold)
{
  const auto target_value = 40.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter = NotEqualFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset = pludux::Asset("");

  EXPECT_TRUE(filter(asset));
}

TEST(NotEqualFilterTest, TargetEqualToThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 50.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter = NotEqualFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset = pludux::Asset("");

  EXPECT_FALSE(filter(asset));
}

TEST(NotEqualFilterTest, TargetGreaterThanThreshold)
{
  const auto target_value = 50.0;
  const auto threshold_value = 20.0;
  auto target_method = ValueMethod{target_value};
  auto threshold_method = ValueMethod{threshold_value};
  const auto filter = NotEqualFilter{std::move(target_method), std::move(threshold_method)};
  const auto asset = pludux::Asset("");

  EXPECT_TRUE(filter(asset));
}

