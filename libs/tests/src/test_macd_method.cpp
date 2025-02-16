#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/data_method.hpp>
#include <pludux/screener/macd_method.hpp>
#include <pludux/series.hpp>

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

  auto macd_method = MacdMethod{
   data_method, MacdOutput::macd, fast_period, slow_period, signal_period};
  auto macd_series = MacdSeries{data_method(asset_data),
                                MacdOutput::macd,
                                fast_period,
                                slow_period,
                                signal_period};

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

  macd_method.output(MacdOutput::signal);
  macd_series.output(MacdOutput::signal);
  const auto signal_lines = macd_method(asset_data);
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

  macd_method.output(MacdOutput::histogram);
  macd_series.output(MacdOutput::histogram);
  const auto histograms = macd_method(asset_data);
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
