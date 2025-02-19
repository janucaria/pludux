#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/boolean_filter.hpp>

using namespace pludux::screener;
using pludux::AssetHistory;

TEST(BooleanFilterTest, TrueFilter)
{
  const auto filter = TrueFilter{};
  const auto asset_data = AssetHistory{};

  EXPECT_TRUE(filter(asset_data));
}

TEST(BooleanFilterTest, FalseFilter)
{
  const auto filter = FalseFilter{};
  const auto asset_data = AssetHistory{};

  EXPECT_FALSE(filter(asset_data));
}