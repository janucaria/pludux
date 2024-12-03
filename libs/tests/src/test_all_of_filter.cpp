#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(AllOfFilterTest, AllFiltersPass)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20}, ValueMethod{10}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30}, ValueMethod{50}};
  const auto all_of_filter =
   AllOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset = pludux::Asset{""};
  const auto asset_data = AssetDataProvider{asset};

  EXPECT_TRUE(all_of_filter(asset_data));
}

TEST(AllOfFilterTest, OneFilterFails)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20}, ValueMethod{30}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30}, ValueMethod{50}};
  const auto all_of_filter =
   AllOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset = pludux::Asset{""};
  const auto asset_data = AssetDataProvider{asset};

  EXPECT_FALSE(all_of_filter(asset_data));
}

TEST(AllOfFilterTest, NoFilters)
{
  const auto all_of_filter = AllOfFilter{};
  const auto asset = pludux::Asset{""};
  const auto asset_data = AssetDataProvider{asset};

  EXPECT_TRUE(all_of_filter(asset_data));
}
