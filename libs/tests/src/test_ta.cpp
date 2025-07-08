import pludux.series;

#include <gtest/gtest.h>
#include <limits>
#include <pludux/ta.hpp>
#include <vector>

using namespace pludux;

TEST(TATest, SmaSeriesCalculation)
{
  const auto series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};
  const auto period = 5;
  const auto result = ta::sma(series, period);

  EXPECT_EQ(result.size(), series.size());
  EXPECT_DOUBLE_EQ(result[0], 862);
  EXPECT_DOUBLE_EQ(result[1], 865);
  EXPECT_DOUBLE_EQ(result[2], 860);
  EXPECT_DOUBLE_EQ(result[3], 848);
  EXPECT_DOUBLE_EQ(result[4], 842);
  EXPECT_DOUBLE_EQ(result[5], 842);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(TATest, EmaSeriesCalculation)
{
  const auto series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};
  const auto period = 5;
  const auto result = ta::ema(series, period);

  EXPECT_EQ(result.size(), series.size());
  EXPECT_DOUBLE_EQ(result[0], 856.95061728395069);
  EXPECT_DOUBLE_EQ(result[1], 857.92592592592598);
  EXPECT_DOUBLE_EQ(result[2], 856.88888888888891);
  EXPECT_DOUBLE_EQ(result[3], 855.33333333333337);
  EXPECT_DOUBLE_EQ(result[4], 853);
  EXPECT_DOUBLE_EQ(result[5], 842);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(TATest, RmaSeriesCalculation)
{
  const auto series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};
  const auto period = 5;
  const auto result = ta::rma(series, period);

  EXPECT_EQ(result.size(), series.size());
  EXPECT_DOUBLE_EQ(result[0], 854.33056000000022);
  EXPECT_DOUBLE_EQ(result[1], 854.16320000000019);
  EXPECT_DOUBLE_EQ(result[2], 852.70400000000018);
  EXPECT_DOUBLE_EQ(result[3], 850.88000000000011);
  EXPECT_DOUBLE_EQ(result[4], 848.60000000000002);
  EXPECT_DOUBLE_EQ(result[5], 842);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(TATest, WMASeriesCalculation)
{
  const auto series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};
  const auto period = 5;
  const auto result = ta::wma(series, period);

  EXPECT_EQ(result.size(), series.size());
  EXPECT_DOUBLE_EQ(result[0], 859.33333333333337);
  EXPECT_DOUBLE_EQ(result[1], 862.66666666666663);
  EXPECT_DOUBLE_EQ(result[2], 862.66666666666663);
  EXPECT_DOUBLE_EQ(result[3], 858.66666666666663);
  EXPECT_DOUBLE_EQ(result[4], 852.66666666666663);
  EXPECT_DOUBLE_EQ(result[5], 841.66666666666663);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(TATest, HmaSeriesCalculation)
{
  const auto series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};
  const auto period = 5;
  const auto result = ta::hma(series, period);

  EXPECT_EQ(result.size(), series.size());
  EXPECT_DOUBLE_EQ(result[0], 855.11111111111097);
  EXPECT_DOUBLE_EQ(result[1], 857.33333333333337);
  EXPECT_DOUBLE_EQ(result[2], 862);
  EXPECT_DOUBLE_EQ(result[3], 878.88888888888903);
  EXPECT_DOUBLE_EQ(result[4], 887.66666666666686);
  EXPECT_TRUE(std::isnan(result[5]));
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(TATest, ChangeSeriesCalculation)
{
  const auto series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};
  const auto result = ta::changes(series);

  EXPECT_EQ(result.size(), series.size());
  EXPECT_DOUBLE_EQ(result[0], -5);
  EXPECT_DOUBLE_EQ(result[1], 0);
  EXPECT_DOUBLE_EQ(result[2], 0);
  EXPECT_DOUBLE_EQ(result[3], -15);
  EXPECT_DOUBLE_EQ(result[4], 5);
  EXPECT_DOUBLE_EQ(result[5], 35);
  EXPECT_DOUBLE_EQ(result[6], 35);
  EXPECT_DOUBLE_EQ(result[7], -30);
  EXPECT_DOUBLE_EQ(result[8], -45);
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(TATest, RsiSeriesCalculation)
{
  const auto series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};
  const auto period = 5;
  const auto result = ta::rsi(series, period);

  EXPECT_EQ(result.size(), series.size());
  EXPECT_DOUBLE_EQ(result[0], 41.446303291958991);
  EXPECT_DOUBLE_EQ(result[1], 44.444444444444443);
  EXPECT_DOUBLE_EQ(result[2], 44.44444444444445);
  EXPECT_DOUBLE_EQ(result[3], 44.444444444444443);
  EXPECT_DOUBLE_EQ(result[4], 50);
  EXPECT_TRUE(std::isnan(result[5]));
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(TATest, TrSeriesCalculation)
{
  const auto highs_series =
   DataSeries<double>{865, 865, 875, 880, 875, 875, 840, 840, 875, 925};
  const auto lows_series =
   DataSeries<double>{850, 850, 855, 845, 855, 820, 800, 800, 830, 815};
  const auto closes_series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};

  const auto result = ta::tr(highs_series, lows_series, closes_series);

  EXPECT_EQ(result.size(), highs_series.size());
  EXPECT_EQ(result.size(), lows_series.size());
  EXPECT_EQ(result.size(), closes_series.size());
  EXPECT_DOUBLE_EQ(result[0], 15);
  EXPECT_DOUBLE_EQ(result[1], 15);
  EXPECT_DOUBLE_EQ(result[2], 20);
  EXPECT_DOUBLE_EQ(result[3], 35);
  EXPECT_DOUBLE_EQ(result[4], 20);
  EXPECT_DOUBLE_EQ(result[5], 55);
  EXPECT_DOUBLE_EQ(result[6], 40);
  EXPECT_DOUBLE_EQ(result[7], 40);
  EXPECT_DOUBLE_EQ(result[8], 45);
  EXPECT_DOUBLE_EQ(result[9], 110);
}

TEST(TATest, AtrSeriesCalculation)
{
  const auto highs_series =
   DataSeries<double>{865, 865, 875, 880, 875, 875, 840, 840, 875, 925};
  const auto lows_series =
   DataSeries<double>{850, 850, 855, 845, 855, 820, 800, 800, 830, 815};
  const auto closes_series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};

  const auto period = 5;
  const auto result = ta::atr(highs_series, lows_series, closes_series, period);

  ASSERT_EQ(result.size(), highs_series.size());
  ASSERT_EQ(result.size(), lows_series.size());
  ASSERT_EQ(result.size(), closes_series.size());
  EXPECT_DOUBLE_EQ(result[0], 32.187840000000008);
  EXPECT_DOUBLE_EQ(result[1], 36.484800000000007);
  EXPECT_DOUBLE_EQ(result[2], 41.856000000000009);
  EXPECT_DOUBLE_EQ(result[3], 47.320000000000007);
  EXPECT_DOUBLE_EQ(result[4], 50.400000000000006);
  EXPECT_DOUBLE_EQ(result[5], 58);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(TATest, MacdSeriesCalculation)
{
  const auto series = DataSeries<double>{
   790, 810, 825, 840, 860, 855, 860, 860, 860, 875, 870, 835, 800, 830, 875};
  const auto short_period = 5;
  const auto long_period = 10;
  const auto signal_period = 3;
  auto macd_series = ta::macd(series, short_period, long_period, signal_period);

  ASSERT_EQ(macd_series.size(), series.size());

  const auto& macd_lines = macd_series;
  EXPECT_DOUBLE_EQ(macd_lines[0], -12.656409004137572);
  EXPECT_DOUBLE_EQ(macd_lines[1], -7.6929729699789959);
  EXPECT_DOUBLE_EQ(macd_lines[2], -3.2941210218015158);
  EXPECT_DOUBLE_EQ(macd_lines[3], 0.969787663390548);
  EXPECT_DOUBLE_EQ(macd_lines[4], 4.5125327347551547);
  EXPECT_DOUBLE_EQ(macd_lines[5], 4.9506172839506917);
  EXPECT_TRUE(std::isnan(macd_lines[10]));
  EXPECT_TRUE(std::isnan(macd_lines[11]));
  EXPECT_TRUE(std::isnan(macd_lines[12]));
  EXPECT_TRUE(std::isnan(macd_lines[13]));
  EXPECT_TRUE(std::isnan(macd_lines[14]));
  EXPECT_TRUE(std::isnan(macd_lines[6]));
  EXPECT_TRUE(std::isnan(macd_lines[7]));
  EXPECT_TRUE(std::isnan(macd_lines[8]));
  EXPECT_TRUE(std::isnan(macd_lines[9]));

  macd_series.output(MacdOutput::signal);
  const auto& signal_lines = macd_series;
  EXPECT_DOUBLE_EQ(signal_lines[0], -8.2285071355347075);
  EXPECT_DOUBLE_EQ(signal_lines[1], -3.8006052669318442);
  EXPECT_DOUBLE_EQ(signal_lines[2], 0.091762436115307766);
  EXPECT_DOUBLE_EQ(signal_lines[3], 3.4776458940321313);
  EXPECT_TRUE(std::isnan(signal_lines[10]));
  EXPECT_TRUE(std::isnan(signal_lines[11]));
  EXPECT_TRUE(std::isnan(signal_lines[12]));
  EXPECT_TRUE(std::isnan(signal_lines[13]));
  EXPECT_TRUE(std::isnan(signal_lines[14]));
  EXPECT_TRUE(std::isnan(signal_lines[4]));
  EXPECT_TRUE(std::isnan(signal_lines[5]));
  EXPECT_TRUE(std::isnan(signal_lines[6]));
  EXPECT_TRUE(std::isnan(signal_lines[7]));
  EXPECT_TRUE(std::isnan(signal_lines[8]));
  EXPECT_TRUE(std::isnan(signal_lines[9]));

  macd_series.output(MacdOutput::histogram);
  const auto& histograms = macd_series;
  EXPECT_DOUBLE_EQ(histograms[0], -4.4279018686028646);
  EXPECT_DOUBLE_EQ(histograms[1], -3.8923677030471517);
  EXPECT_DOUBLE_EQ(histograms[2], -3.3858834579168233);
  EXPECT_DOUBLE_EQ(histograms[3], -2.5078582306415833);
  EXPECT_TRUE(std::isnan(histograms[10]));
  EXPECT_TRUE(std::isnan(histograms[11]));
  EXPECT_TRUE(std::isnan(histograms[12]));
  EXPECT_TRUE(std::isnan(histograms[13]));
  EXPECT_TRUE(std::isnan(histograms[14]));
  EXPECT_TRUE(std::isnan(histograms[4]));
  EXPECT_TRUE(std::isnan(histograms[5]));
  EXPECT_TRUE(std::isnan(histograms[6]));
  EXPECT_TRUE(std::isnan(histograms[7]));
  EXPECT_TRUE(std::isnan(histograms[8]));
  EXPECT_TRUE(std::isnan(histograms[9]));
}

TEST(TATest, StochSeriesCalculation)
{
  const auto highs_series =
   DataSeries<double>{865, 865, 875, 880, 875, 875, 840, 840, 875, 925};
  const auto lows_series =
   DataSeries<double>{850, 850, 855, 845, 855, 820, 800, 800, 830, 815};
  const auto closes_series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};

  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 3;
  auto stochastic = ta::stoch(
   highs_series, lows_series, closes_series, k_period, k_smooth, d_period);
  const auto k = stochastic;

  stochastic.output(StochOutput::d);
  const auto d = stochastic;

  ASSERT_EQ(k.size(), highs_series.size());
  ASSERT_EQ(d.size(), highs_series.size());

  EXPECT_DOUBLE_EQ(k[0], 56.746031746031754);
  EXPECT_DOUBLE_EQ(k[1], 72.222222222222229);
  EXPECT_DOUBLE_EQ(k[2], 83.333333333333329);
  EXPECT_DOUBLE_EQ(k[3], 77);
  EXPECT_TRUE(std::isnan(k[4]));
  EXPECT_TRUE(std::isnan(k[5]));
  EXPECT_TRUE(std::isnan(k[6]));
  EXPECT_TRUE(std::isnan(k[7]));
  EXPECT_TRUE(std::isnan(k[8]));
  EXPECT_TRUE(std::isnan(k[9]));

  EXPECT_DOUBLE_EQ(d[0], 70.767195767195759);
  EXPECT_DOUBLE_EQ(d[1], 77.518518518518519);
  EXPECT_TRUE(std::isnan(d[2]));
  EXPECT_TRUE(std::isnan(d[3]));
  EXPECT_TRUE(std::isnan(d[4]));
  EXPECT_TRUE(std::isnan(d[5]));
  EXPECT_TRUE(std::isnan(d[6]));
  EXPECT_TRUE(std::isnan(d[7]));
  EXPECT_TRUE(std::isnan(d[8]));
  EXPECT_TRUE(std::isnan(d[9]));
}

TEST(TATest, StochRSISeriesCalculation)
{
  const auto rsi_input = DataSeries<double>{
   790, 810, 825, 840, 860, 855, 860, 860, 860, 875, 870, 835, 800, 830, 875};
  const auto rsi_period = 5;
  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 3;
  auto result =
   ta::stoch_rsi(rsi_input, rsi_period, k_period, k_smooth, d_period);

  const auto k = result;

  result.output(StochOutput::d);
  const auto d = result;

  ASSERT_EQ(k.size(), rsi_input.size());
  ASSERT_EQ(d.size(), rsi_input.size());

  EXPECT_DOUBLE_EQ(k[0], 0);
  EXPECT_DOUBLE_EQ(k[1], 0);
  EXPECT_DOUBLE_EQ(k[2], 33.333333333333336);
  EXPECT_DOUBLE_EQ(k[3], 33.333333333333336);
  EXPECT_DOUBLE_EQ(k[4], 33.333333333333336);
  EXPECT_DOUBLE_EQ(k[5], 4.2632564145606005e-14);
  EXPECT_DOUBLE_EQ(k[6], 4.2632564145606005e-14);
  EXPECT_TRUE(std::isnan(k[7]));
  EXPECT_TRUE(std::isnan(k[8]));
  EXPECT_TRUE(std::isnan(k[9]));
  EXPECT_TRUE(std::isnan(k[10]));
  EXPECT_TRUE(std::isnan(k[11]));
  EXPECT_TRUE(std::isnan(k[12]));
  EXPECT_TRUE(std::isnan(k[13]));
  EXPECT_TRUE(std::isnan(k[14]));

  EXPECT_DOUBLE_EQ(d[0], 11.111111111111111);
  EXPECT_DOUBLE_EQ(d[1], 22.222222222222221);
  EXPECT_DOUBLE_EQ(d[2], 33.333333333333336);
  EXPECT_DOUBLE_EQ(d[3], 22.222222222222239);
  EXPECT_DOUBLE_EQ(d[4], 11.111111111111141);
  EXPECT_TRUE(std::isnan(d[5]));
  EXPECT_TRUE(std::isnan(d[6]));
  EXPECT_TRUE(std::isnan(d[7]));
  EXPECT_TRUE(std::isnan(d[8]));
  EXPECT_TRUE(std::isnan(d[9]));
  EXPECT_TRUE(std::isnan(d[10]));
  EXPECT_TRUE(std::isnan(d[11]));
  EXPECT_TRUE(std::isnan(d[12]));
  EXPECT_TRUE(std::isnan(d[13]));
  EXPECT_TRUE(std::isnan(d[14]));
}

TEST(TATest, BbSeriesCalculation)
{
  const auto series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};
  const auto period = 5;
  const auto stddev = 2;
  auto result = ta::bb(series, period, stddev);

  ASSERT_EQ(result.size(), series.size());

  const auto& middle_band = result;
  EXPECT_DOUBLE_EQ(middle_band[0], 862);
  EXPECT_DOUBLE_EQ(middle_band[1], 865);
  EXPECT_DOUBLE_EQ(middle_band[2], 860);
  EXPECT_DOUBLE_EQ(middle_band[3], 848);
  EXPECT_DOUBLE_EQ(middle_band[4], 842);
  EXPECT_DOUBLE_EQ(middle_band[5], 842);
  EXPECT_TRUE(std::isnan(middle_band[6]));
  EXPECT_TRUE(std::isnan(middle_band[7]));
  EXPECT_TRUE(std::isnan(middle_band[8]));
  EXPECT_TRUE(std::isnan(middle_band[9]));

  result.output(BbOutput::upper);
  const auto& upper_band = result;
  EXPECT_DOUBLE_EQ(upper_band[0], 875.56465996625059);
  EXPECT_DOUBLE_EQ(upper_band[1], 877.64911064067348);
  EXPECT_DOUBLE_EQ(upper_band[2], 887.5680975041804);
  EXPECT_DOUBLE_EQ(upper_band[3], 903.35341001239215);
  EXPECT_DOUBLE_EQ(upper_band[4], 897.35341001239215);
  EXPECT_DOUBLE_EQ(upper_band[5], 897.35341001239215);
  EXPECT_TRUE(std::isnan(upper_band[6]));
  EXPECT_TRUE(std::isnan(upper_band[7]));
  EXPECT_TRUE(std::isnan(upper_band[8]));
  EXPECT_TRUE(std::isnan(upper_band[9]));

  result.output(BbOutput::lower);
  const auto& lower_band = result;
  EXPECT_DOUBLE_EQ(lower_band[0], 848.43534003374941);
  EXPECT_DOUBLE_EQ(lower_band[1], 852.35088935932652);
  EXPECT_DOUBLE_EQ(lower_band[2], 832.4319024958196);
  EXPECT_DOUBLE_EQ(lower_band[3], 792.64658998760785);
  EXPECT_DOUBLE_EQ(lower_band[4], 786.64658998760785);
  EXPECT_DOUBLE_EQ(lower_band[5], 786.64658998760785);
  EXPECT_TRUE(std::isnan(lower_band[6]));
  EXPECT_TRUE(std::isnan(lower_band[7]));
  EXPECT_TRUE(std::isnan(lower_band[8]));
  EXPECT_TRUE(std::isnan(lower_band[9]));
}

TEST(TATest, PivotLowsSeriesCalculation)
{
  const auto series =
   DataSeries<double>{855, 860, 860, 860, 875, 870, 835, 800, 830, 875};
  const auto range = 2;
  const auto result = ta::pivot_lows(series, range);

  ASSERT_EQ(result.size(), series.size());
  EXPECT_TRUE(std::isnan(result[0]));
  EXPECT_TRUE(std::isnan(result[1]));
  EXPECT_TRUE(std::isnan(result[2]));
  EXPECT_TRUE(std::isnan(result[3]));
  EXPECT_TRUE(std::isnan(result[4]));
  EXPECT_TRUE(std::isnan(result[5]));
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_DOUBLE_EQ(result[7], 800);
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}