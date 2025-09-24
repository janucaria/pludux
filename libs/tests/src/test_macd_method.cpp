#include <gtest/gtest.h>

#include <cmath>

import pludux;

using namespace pludux;
using namespace pludux::screener;

TEST(MacdMethodTest, RunAllMethod)
{
  const auto data_method = DataMethod{"close"};
  const auto fast_period = 5;
  const auto slow_period = 10;
  const auto signal_period = 3;
  const auto asset_data = pludux::AssetHistory{{"close",
                                                {790,
                                                 810,
                                                 825,
                                                 840,
                                                 860,
                                                 855,
                                                 860,
                                                 860,
                                                 860,
                                                 875,
                                                 870,
                                                 835,
                                                 800,
                                                 830,
                                                 875}}};

  auto macd_method =
   MacdMethod{data_method, fast_period, slow_period, signal_period};
  auto macd_series = OutputByNameSeries{
   MacdSeries{data_method(asset_data), fast_period, slow_period, signal_period},
   OutputName::MacdLine};

  const auto macd_lines = macd_method(asset_data);
  ASSERT_EQ(macd_lines.size(), macd_series.size());
  EXPECT_DOUBLE_EQ(macd_lines[0], macd_series[0]);
  EXPECT_DOUBLE_EQ(macd_lines[1], macd_series[1]);
  EXPECT_DOUBLE_EQ(macd_lines[2], macd_series[2]);
  EXPECT_DOUBLE_EQ(macd_lines[3], macd_series[3]);
  EXPECT_DOUBLE_EQ(macd_lines[4], macd_series[4]);
  EXPECT_DOUBLE_EQ(macd_lines[5], macd_series[5]);
  EXPECT_TRUE(std::isnan(macd_lines[10]) && macd_series[10]);
  EXPECT_TRUE(std::isnan(macd_lines[11]) && macd_series[11]);
  EXPECT_TRUE(std::isnan(macd_lines[12]) && macd_series[12]);
  EXPECT_TRUE(std::isnan(macd_lines[13]) && macd_series[13]);
  EXPECT_TRUE(std::isnan(macd_lines[14]) && macd_series[14]);
  EXPECT_TRUE(std::isnan(macd_lines[6]) && macd_series[6]);
  EXPECT_TRUE(std::isnan(macd_lines[7]) && macd_series[7]);
  EXPECT_TRUE(std::isnan(macd_lines[8]) && macd_series[8]);
  EXPECT_TRUE(std::isnan(macd_lines[9]) && macd_series[9]);

  macd_series.output_name(OutputName::SignalLine);
  const auto signal_lines =
   OutputByNameMethod{macd_method, OutputName::SignalLine}(asset_data);
  EXPECT_DOUBLE_EQ(signal_lines[0], macd_series[0]);
  EXPECT_DOUBLE_EQ(signal_lines[1], macd_series[1]);
  EXPECT_DOUBLE_EQ(signal_lines[2], macd_series[2]);
  EXPECT_DOUBLE_EQ(signal_lines[3], macd_series[3]);
  EXPECT_TRUE(std::isnan(signal_lines[10]) && macd_series[10]);
  EXPECT_TRUE(std::isnan(signal_lines[11]) && macd_series[11]);
  EXPECT_TRUE(std::isnan(signal_lines[12]) && macd_series[12]);
  EXPECT_TRUE(std::isnan(signal_lines[13]) && macd_series[13]);
  EXPECT_TRUE(std::isnan(signal_lines[14]) && macd_series[14]);
  EXPECT_TRUE(std::isnan(signal_lines[4]) && macd_series[4]);
  EXPECT_TRUE(std::isnan(signal_lines[5]) && macd_series[5]);
  EXPECT_TRUE(std::isnan(signal_lines[6]) && macd_series[6]);
  EXPECT_TRUE(std::isnan(signal_lines[7]) && macd_series[7]);
  EXPECT_TRUE(std::isnan(signal_lines[8]) && macd_series[8]);
  EXPECT_TRUE(std::isnan(signal_lines[9]) && macd_series[9]);

  macd_series.output_name(OutputName::MacdHistogram);
  const auto histograms =
   OutputByNameMethod{macd_method, OutputName::MacdHistogram}(asset_data);
  EXPECT_DOUBLE_EQ(histograms[0], macd_series[0]);
  EXPECT_DOUBLE_EQ(histograms[1], macd_series[1]);
  EXPECT_DOUBLE_EQ(histograms[2], macd_series[2]);
  EXPECT_DOUBLE_EQ(histograms[3], macd_series[3]);
  EXPECT_TRUE(std::isnan(histograms[10]) && macd_series[10]);
  EXPECT_TRUE(std::isnan(histograms[11]) && macd_series[11]);
  EXPECT_TRUE(std::isnan(histograms[12]) && macd_series[12]);
  EXPECT_TRUE(std::isnan(histograms[13]) && macd_series[13]);
  EXPECT_TRUE(std::isnan(histograms[14]) && macd_series[14]);
  EXPECT_TRUE(std::isnan(histograms[4]) && macd_series[4]);
  EXPECT_TRUE(std::isnan(histograms[5]) && macd_series[5]);
  EXPECT_TRUE(std::isnan(histograms[6]) && macd_series[6]);
  EXPECT_TRUE(std::isnan(histograms[7]) && macd_series[7]);
  EXPECT_TRUE(std::isnan(histograms[8]) && macd_series[8]);
  EXPECT_TRUE(std::isnan(histograms[9]) && macd_series[9]);
}

TEST(MacdMethodTest, EqualityOperator)
{
  const auto data_method1 = DataMethod{"close"};
  const auto fast_period1 = 5;
  const auto slow_period1 = 10;
  const auto signal_period1 = 3;
  const auto macd_method1 =
   MacdMethod{data_method1, fast_period1, slow_period1, signal_period1};

  const auto data_method2 = DataMethod{"close"};
  const auto fast_period2 = 5;
  const auto slow_period2 = 10;
  const auto signal_period2 = 3;
  const auto macd_method2 =
   MacdMethod{data_method2, fast_period2, slow_period2, signal_period2};

  EXPECT_TRUE(macd_method1 == macd_method2);
  EXPECT_FALSE(macd_method1 != macd_method2);
}

TEST(MacdMethodTest, NotEqualOperator)
{
  const auto data_method1 = DataMethod{"close"};
  const auto fast_period1 = 5;
  const auto slow_period1 = 10;
  const auto signal_period1 = 3;
  const auto macd_method1 =
   MacdMethod{data_method1, fast_period1, slow_period1, signal_period1};

  const auto data_method2 = DataMethod{"open"};
  const auto fast_period2 = 5;
  const auto slow_period2 = 10;
  const auto signal_period2 = 3;
  const auto macd_method2 =
   MacdMethod{data_method2, fast_period2, slow_period2, signal_period2};

  EXPECT_TRUE(macd_method1 != macd_method2);
  EXPECT_FALSE(macd_method1 == macd_method2);
}
