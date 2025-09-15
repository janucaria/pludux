#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/lookback_method.hpp>
#include <pludux/screener/ohlcv_method.hpp>
#include <pludux/series.hpp>
#include <stdexcept>

using namespace pludux::screener;

TEST(LookbackMethodTest, RunAllMethodClose)
{
  const auto asset_data = pludux::AssetHistory{{"Close", {4.0, 4.1, 4.2}}};
  const auto lookback_method = LookbackMethod{CloseMethod{}, 1};

  EXPECT_EQ(lookback_method.period(), 1);
  EXPECT_EQ(lookback_method.source(), CloseMethod{});

  const auto series = lookback_method(asset_data);
  EXPECT_EQ(series.size(), 2);
  EXPECT_EQ(series[0], 4.1);
  EXPECT_EQ(series[1], 4.2);
  EXPECT_TRUE(std::isnan(series[2]));
}

TEST(LookbackMethodTest, EqualityComparisonOperator)
{
  const auto lookback_method1 = LookbackMethod{CloseMethod{}, 1};
  const auto lookback_method2 = LookbackMethod{CloseMethod{}, 1};
  const auto lookback_method3 = LookbackMethod{CloseMethod{}, 2};

  EXPECT_EQ(lookback_method1, lookback_method2);
  EXPECT_NE(lookback_method1, lookback_method3);
  EXPECT_NE(lookback_method2, lookback_method3);
}