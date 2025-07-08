import pludux.series;

#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener.hpp>

using namespace pludux::screener;

TEST(ScreenerFilterTest, RunOneFilter)
{
  const auto equal_filter = EqualFilter{ValueMethod{1.0}, ValueMethod{1.0}};
  const auto screener_filter = ScreenerFilter{equal_filter};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};
  const auto result = screener_filter(asset_data);
  EXPECT_TRUE(result);

  const auto casted_filter = screener_filter_cast<EqualFilter>(screener_filter);
  ASSERT_NE(casted_filter, nullptr);
}

TEST(ScreenerFilterTest, EqualityOperator)
{
  const auto filter1 = TrueFilter{};
  const auto filter2 = TrueFilter{};
  const auto screener_filter1 = ScreenerFilter{filter1};
  const auto screener_filter2 = ScreenerFilter{filter2};

  EXPECT_TRUE(screener_filter1 == screener_filter2);
  EXPECT_FALSE(screener_filter1 != screener_filter2);
  EXPECT_EQ(screener_filter1, screener_filter2);
}

TEST(ScreenerFilterTest, NotEqualOperator)
{
  const auto filter1 = TrueFilter{};
  const auto filter2 = FalseFilter{};
  const auto screener_filter1 = ScreenerFilter{filter1};
  const auto screener_filter2 = ScreenerFilter{filter2};

  EXPECT_TRUE(screener_filter1 != screener_filter2);
  EXPECT_FALSE(screener_filter1 == screener_filter2);
}