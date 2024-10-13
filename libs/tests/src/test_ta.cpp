#include <gtest/gtest.h>
#include <limits>
#include <pludux/quote.hpp>
#include <pludux/ta.hpp>
#include <vector>

using namespace pludux;
#include <gtest/gtest.h>
#include <limits>
#include <pludux/quote.hpp>
#include <pludux/ta.hpp>
#include <vector>

using namespace pludux;

TEST(TATest, SMACalculation)
{
  const auto sources =
   std::vector<double>{875, 830, 800, 835, 870, 875, 860, 860, 860, 855};

  const auto period = 5;
  const auto result = ta::sma(sources, period);

  ASSERT_EQ(result.size(), sources.size());
  EXPECT_TRUE(std::isnan(result[0]));
  EXPECT_TRUE(std::isnan(result[1]));
  EXPECT_TRUE(std::isnan(result[2]));
  EXPECT_TRUE(std::isnan(result[3]));
  EXPECT_DOUBLE_EQ(result[4], 842);
  EXPECT_DOUBLE_EQ(result[5], 842);
  EXPECT_DOUBLE_EQ(result[6], 848);
  EXPECT_DOUBLE_EQ(result[7], 860);
  EXPECT_DOUBLE_EQ(result[8], 865);
  EXPECT_DOUBLE_EQ(result[9], 862);
}

TEST(TATest, EMACalculation)
{
  const auto sources =
   std::vector<double>{875, 830, 800, 835, 870, 875, 860, 860, 860, 855};
  const auto period = 5;
  const auto result = ta::ema(sources, period);

  ASSERT_EQ(result.size(), sources.size());
  EXPECT_TRUE(std::isnan(result[0]));
  EXPECT_TRUE(std::isnan(result[1]));
  EXPECT_TRUE(std::isnan(result[2]));
  EXPECT_TRUE(std::isnan(result[3]));
  EXPECT_DOUBLE_EQ(result[4], 842);
  EXPECT_DOUBLE_EQ(result[5], 853);
  EXPECT_DOUBLE_EQ(result[6], 855.33333333333337);
  EXPECT_DOUBLE_EQ(result[7], 856.88888888888891);
  EXPECT_DOUBLE_EQ(result[8], 857.92592592592598);
  EXPECT_DOUBLE_EQ(result[9], 856.95061728395069);
}

TEST(TATest, RMACalculation)
{
  const auto sources =
   std::vector<double>{875, 830, 800, 835, 870, 875, 860, 860, 860, 855};
  const auto period = 5;
  const auto result = ta::rma(sources, period);

  ASSERT_EQ(result.size(), sources.size());
  EXPECT_TRUE(std::isnan(result[0]));
  EXPECT_TRUE(std::isnan(result[1]));
  EXPECT_TRUE(std::isnan(result[2]));
  EXPECT_TRUE(std::isnan(result[3]));
  EXPECT_DOUBLE_EQ(result[4], 842);
  EXPECT_DOUBLE_EQ(result[5], 848.60000000000002);
  EXPECT_DOUBLE_EQ(result[6], 850.88000000000011);
  EXPECT_DOUBLE_EQ(result[7], 852.70400000000018);
  EXPECT_DOUBLE_EQ(result[8], 854.16320000000019);
  EXPECT_DOUBLE_EQ(result[9], 854.33056000000022);
}

TEST(TATest, WMACalculation)
{
  const auto sources =
   std::vector<double>{875, 830, 800, 835, 870, 875, 860, 860, 860, 855};
  const auto period = 5;
  const auto result = ta::wma(sources, period);

  ASSERT_EQ(result.size(), sources.size());
  EXPECT_TRUE(std::isnan(result[0]));
  EXPECT_TRUE(std::isnan(result[1]));
  EXPECT_TRUE(std::isnan(result[2]));
  EXPECT_TRUE(std::isnan(result[3]));
  EXPECT_DOUBLE_EQ(result[4], 841.66666666666663);
  EXPECT_DOUBLE_EQ(result[5], 852.66666666666663);
  EXPECT_DOUBLE_EQ(result[6], 858.66666666666663);
  EXPECT_DOUBLE_EQ(result[7], 862.66666666666663);
  EXPECT_DOUBLE_EQ(result[8], 862.66666666666663);
  EXPECT_DOUBLE_EQ(result[9], 859.33333333333337);
}

TEST(TATest, HMACalculation)
{
  const auto sources =
   std::vector<double>{875, 830, 800, 835, 870, 875, 860, 860, 860, 855};
  const auto period = 5;
  const auto result = ta::hma(sources, period);

  ASSERT_EQ(result.size(), sources.size());
  EXPECT_TRUE(std::isnan(result[0]));
  EXPECT_TRUE(std::isnan(result[1]));
  EXPECT_TRUE(std::isnan(result[2]));
  EXPECT_TRUE(std::isnan(result[3]));
  EXPECT_TRUE(std::isnan(result[4]));
  EXPECT_DOUBLE_EQ(result[5], 887.66666666666686);
  EXPECT_DOUBLE_EQ(result[6], 878.88888888888903);
  EXPECT_DOUBLE_EQ(result[7], 862);
  EXPECT_DOUBLE_EQ(result[8], 857.33333333333337);
  EXPECT_DOUBLE_EQ(result[9], 855.11111111111097);
}

TEST(TATest, ChangesCalculation)
{
  const auto sources =
   std::vector<double>{875, 830, 800, 835, 870, 875, 860, 860, 860, 855};
  const auto result = ta::changes(sources);

  ASSERT_EQ(result.size(), sources.size());
  EXPECT_TRUE(std::isnan(result[0]));
  EXPECT_DOUBLE_EQ(result[1], -45);
  EXPECT_DOUBLE_EQ(result[2], -30);
  EXPECT_DOUBLE_EQ(result[3], 35);
  EXPECT_DOUBLE_EQ(result[4], 35);
  EXPECT_DOUBLE_EQ(result[5], 5);
  EXPECT_DOUBLE_EQ(result[6], -15);
  EXPECT_DOUBLE_EQ(result[7], 0);
  EXPECT_DOUBLE_EQ(result[8], 0);
  EXPECT_DOUBLE_EQ(result[9], -5);
}

TEST(TATest, RSICalculation)
{
  const auto sources =
   std::vector<double>{875, 830, 800, 835, 870, 875, 860, 860, 860, 855};
  const auto period = 5;
  const auto result = ta::rsi(sources, period);

  ASSERT_EQ(result.size(), sources.size());
  EXPECT_TRUE(std::isnan(result[0]));
  EXPECT_TRUE(std::isnan(result[1]));
  EXPECT_TRUE(std::isnan(result[2]));
  EXPECT_TRUE(std::isnan(result[3]));
  EXPECT_TRUE(std::isnan(result[4]));
  EXPECT_DOUBLE_EQ(result[5], 50);
  EXPECT_DOUBLE_EQ(result[6], 44.444444444444443);
  EXPECT_DOUBLE_EQ(result[7], 44.44444444444445);
  EXPECT_DOUBLE_EQ(result[8], 44.444444444444443);
  EXPECT_DOUBLE_EQ(result[9], 41.446303291958991);
}

TEST(TATest, TRCalculation)
{
  const auto quotes = std::vector<Quote>{Quote{0, 0, 925, 815, 875, 0},
                                         Quote{0, 0, 875, 830, 830, 0},
                                         Quote{0, 0, 840, 800, 800, 0},
                                         Quote{0, 0, 840, 800, 835, 0},
                                         Quote{0, 0, 875, 820, 870, 0},
                                         Quote{0, 0, 875, 855, 875, 0},
                                         Quote{0, 0, 880, 845, 860, 0},
                                         Quote{0, 0, 875, 855, 860, 0},
                                         Quote{0, 0, 865, 850, 860, 0},
                                         Quote{0, 0, 865, 850, 855, 0}};

  const auto result = ta::tr(quotes);

  ASSERT_EQ(result.size(), quotes.size());
  EXPECT_DOUBLE_EQ(result[0], 110);
  EXPECT_DOUBLE_EQ(result[1], 45);
  EXPECT_DOUBLE_EQ(result[2], 40);
  EXPECT_DOUBLE_EQ(result[3], 40);
  EXPECT_DOUBLE_EQ(result[4], 55);
  EXPECT_DOUBLE_EQ(result[5], 20);
  EXPECT_DOUBLE_EQ(result[6], 35);
  EXPECT_DOUBLE_EQ(result[7], 20);
  EXPECT_DOUBLE_EQ(result[8], 15);
  EXPECT_DOUBLE_EQ(result[9], 15);
}

TEST(TATest, ATRCalculation)
{
  const auto quotes = std::vector<Quote>{Quote{0, 925, 925, 815, 875, 100},
                                         Quote{0, 875, 875, 830, 830, 100},
                                         Quote{0, 815, 840, 800, 800, 100},
                                         Quote{0, 800, 840, 800, 835, 100},
                                         Quote{0, 840, 875, 820, 870, 100},
                                         Quote{0, 870, 875, 855, 875, 100},
                                         Quote{0, 870, 880, 845, 860, 100},
                                         Quote{0, 860, 875, 855, 860, 100},
                                         Quote{0, 860, 865, 850, 860, 100},
                                         Quote{0, 860, 865, 850, 855, 100}};

  const auto period = 5;
  const auto result = ta::atr(quotes, period);

  ASSERT_EQ(result.size(), quotes.size());
  EXPECT_TRUE(std::isnan(result[0]));
  EXPECT_TRUE(std::isnan(result[1]));
  EXPECT_TRUE(std::isnan(result[2]));
  EXPECT_TRUE(std::isnan(result[3]));
  EXPECT_DOUBLE_EQ(result[4], 58);
  EXPECT_DOUBLE_EQ(result[5], 50.400000000000006);
  EXPECT_DOUBLE_EQ(result[6], 47.320000000000007);
  EXPECT_DOUBLE_EQ(result[7], 41.856000000000009);
  EXPECT_DOUBLE_EQ(result[8], 36.484800000000007);
  EXPECT_DOUBLE_EQ(result[9], 32.187840000000008);
}

TEST(TATest, MACDCalculation)
{
  const auto sources = std::vector<double>{
   875, 830, 800, 835, 870, 875, 860, 860, 860, 855, 860, 840, 825, 810, 790};

  const auto short_period = 5;
  const auto long_period = 10;
  const auto signal_period = 3;
  const auto [macd_lines, signal_lines, histograms] =
   ta::macd(sources, short_period, long_period, signal_period);

  ASSERT_EQ(macd_lines.size(), sources.size());
  ASSERT_EQ(signal_lines.size(), sources.size());
  ASSERT_EQ(histograms.size(), sources.size());
  EXPECT_TRUE(std::isnan(macd_lines[0]));
  EXPECT_TRUE(std::isnan(macd_lines[1]));
  EXPECT_TRUE(std::isnan(macd_lines[2]));
  EXPECT_TRUE(std::isnan(macd_lines[3]));
  EXPECT_TRUE(std::isnan(macd_lines[4]));
  EXPECT_TRUE(std::isnan(macd_lines[5]));
  EXPECT_TRUE(std::isnan(macd_lines[6]));
  EXPECT_TRUE(std::isnan(macd_lines[7]));
  EXPECT_TRUE(std::isnan(macd_lines[8]));
  EXPECT_DOUBLE_EQ(macd_lines[9], 4.9506172839506917);
  EXPECT_DOUBLE_EQ(macd_lines[10], 4.5125327347551547);
  EXPECT_DOUBLE_EQ(macd_lines[11], 0.969787663390548);
  EXPECT_DOUBLE_EQ(macd_lines[12], -3.2941210218015158);
  EXPECT_DOUBLE_EQ(macd_lines[13], -7.6929729699789959);
  EXPECT_DOUBLE_EQ(macd_lines[14], -12.656409004137572);

  EXPECT_TRUE(std::isnan(signal_lines[0]));
  EXPECT_TRUE(std::isnan(signal_lines[1]));
  EXPECT_TRUE(std::isnan(signal_lines[2]));
  EXPECT_TRUE(std::isnan(signal_lines[3]));
  EXPECT_TRUE(std::isnan(signal_lines[4]));
  EXPECT_TRUE(std::isnan(signal_lines[5]));
  EXPECT_TRUE(std::isnan(signal_lines[6]));
  EXPECT_TRUE(std::isnan(signal_lines[7]));
  EXPECT_TRUE(std::isnan(signal_lines[8]));
  EXPECT_TRUE(std::isnan(signal_lines[9]));
  EXPECT_TRUE(std::isnan(signal_lines[10]));
  EXPECT_DOUBLE_EQ(signal_lines[11], 3.4776458940321313);
  EXPECT_DOUBLE_EQ(signal_lines[12], 0.091762436115307766);
  EXPECT_DOUBLE_EQ(signal_lines[13], -3.8006052669318442);
  EXPECT_DOUBLE_EQ(signal_lines[14], -8.2285071355347075);

  EXPECT_TRUE(std::isnan(histograms[0]));
  EXPECT_TRUE(std::isnan(histograms[1]));
  EXPECT_TRUE(std::isnan(histograms[2]));
  EXPECT_TRUE(std::isnan(histograms[3]));
  EXPECT_TRUE(std::isnan(histograms[4]));
  EXPECT_TRUE(std::isnan(histograms[5]));
  EXPECT_TRUE(std::isnan(histograms[6]));
  EXPECT_TRUE(std::isnan(histograms[7]));
  EXPECT_TRUE(std::isnan(histograms[8]));
  EXPECT_TRUE(std::isnan(histograms[9]));
  EXPECT_TRUE(std::isnan(histograms[10]));
  EXPECT_DOUBLE_EQ(histograms[11], -2.5078582306415833);
  EXPECT_DOUBLE_EQ(histograms[12], -3.3858834579168233);
  EXPECT_DOUBLE_EQ(histograms[13], -3.8923677030471517);
  EXPECT_DOUBLE_EQ(histograms[14], -4.4279018686028646);
}

TEST(TATest, StochCalculation)
{
  const auto quotes = std::vector<Quote>{Quote{0, 0, 925, 815, 875, 0},
                                         Quote{0, 0, 875, 830, 830, 0},
                                         Quote{0, 0, 840, 800, 800, 0},
                                         Quote{0, 0, 840, 800, 835, 0},
                                         Quote{0, 0, 875, 820, 870, 0},
                                         Quote{0, 0, 875, 855, 875, 0},
                                         Quote{0, 0, 880, 845, 860, 0},
                                         Quote{0, 0, 875, 855, 860, 0},
                                         Quote{0, 0, 865, 850, 860, 0},
                                         Quote{0, 0, 865, 850, 855, 0}};

  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 3;
  const auto [k, d] = ta::stoch(quotes, k_period, k_smooth, d_period);

  ASSERT_EQ(k.size(), quotes.size());
  ASSERT_EQ(d.size(), quotes.size());

  EXPECT_TRUE(std::isnan(k[0]));
  EXPECT_TRUE(std::isnan(k[1]));
  EXPECT_TRUE(std::isnan(k[2]));
  EXPECT_TRUE(std::isnan(k[3]));
  EXPECT_TRUE(std::isnan(k[4]));
  EXPECT_TRUE(std::isnan(k[5]));
  EXPECT_DOUBLE_EQ(k[6], 77);
  EXPECT_DOUBLE_EQ(k[7], 83.333333333333329);
  EXPECT_DOUBLE_EQ(k[8], 72.222222222222229);
  EXPECT_DOUBLE_EQ(k[9], 56.746031746031754);

  EXPECT_TRUE(std::isnan(d[0]));
  EXPECT_TRUE(std::isnan(d[1]));
  EXPECT_TRUE(std::isnan(d[2]));
  EXPECT_TRUE(std::isnan(d[3]));
  EXPECT_TRUE(std::isnan(d[4]));
  EXPECT_TRUE(std::isnan(d[5]));
  EXPECT_TRUE(std::isnan(d[6]));
  EXPECT_TRUE(std::isnan(d[7]));
  EXPECT_DOUBLE_EQ(d[8], 77.518518518518519);
  EXPECT_DOUBLE_EQ(d[9], 70.767195767195759);
}

TEST(TATest, StochRSICalculation)
{
  const auto sources = std::vector<double>{
   875, 830, 800, 835, 870, 875, 860, 860, 860, 855, 860, 840, 825, 810, 790};

  const auto rsi_period = 5;
  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 3;
  const auto [k, d] =
   ta::stoch_rsi(sources, rsi_period, k_period, k_smooth, d_period);

  ASSERT_EQ(k.size(), sources.size());
  ASSERT_EQ(d.size(), sources.size());

  EXPECT_TRUE(std::isnan(k[0]));
  EXPECT_TRUE(std::isnan(k[1]));
  EXPECT_TRUE(std::isnan(k[2]));
  EXPECT_TRUE(std::isnan(k[3]));
  EXPECT_TRUE(std::isnan(k[4]));
  EXPECT_TRUE(std::isnan(k[5]));
  EXPECT_TRUE(std::isnan(k[6]));
  EXPECT_TRUE(std::isnan(k[7]));
  EXPECT_DOUBLE_EQ(k[8], 4.2632564145606005e-14);
  EXPECT_DOUBLE_EQ(k[9], 4.2632564145606005e-14);
  EXPECT_DOUBLE_EQ(k[10], 33.333333333333336);
  EXPECT_DOUBLE_EQ(k[11], 33.333333333333336);
  EXPECT_DOUBLE_EQ(k[12], 33.333333333333336);
  EXPECT_DOUBLE_EQ(k[13], 0);
  EXPECT_DOUBLE_EQ(k[14], 0);

  EXPECT_TRUE(std::isnan(d[0]));
  EXPECT_TRUE(std::isnan(d[1]));
  EXPECT_TRUE(std::isnan(d[2]));
  EXPECT_TRUE(std::isnan(d[3]));
  EXPECT_TRUE(std::isnan(d[4]));
  EXPECT_TRUE(std::isnan(d[5]));
  EXPECT_TRUE(std::isnan(d[6]));
  EXPECT_TRUE(std::isnan(d[7]));
  EXPECT_TRUE(std::isnan(d[8]));
  EXPECT_TRUE(std::isnan(d[9]));
  EXPECT_DOUBLE_EQ(d[10], 11.111111111111141);
  EXPECT_DOUBLE_EQ(d[11], 22.222222222222239);
  EXPECT_DOUBLE_EQ(d[12], 33.333333333333336);
  EXPECT_DOUBLE_EQ(d[13], 22.222222222222221);
  EXPECT_DOUBLE_EQ(d[14], 11.111111111111111);
}

TEST(TATest, BBCalculation)
{
  const auto sources =
   std::vector<double>{875, 830, 800, 835, 870, 875, 860, 860, 860, 855};

  const auto period = 5;
  const auto stddev = 2;

  const auto [middle_band, upper_band, lower_band] =
   ta::bb(sources, period, stddev);

  ASSERT_EQ(middle_band.size(), sources.size());
  ASSERT_EQ(upper_band.size(), sources.size());
  ASSERT_EQ(lower_band.size(), sources.size());

  EXPECT_TRUE(std::isnan(middle_band[0]));
  EXPECT_TRUE(std::isnan(middle_band[1]));
  EXPECT_TRUE(std::isnan(middle_band[2]));
  EXPECT_TRUE(std::isnan(middle_band[3]));
  EXPECT_DOUBLE_EQ(middle_band[4], 842);
  EXPECT_DOUBLE_EQ(middle_band[5], 842);
  EXPECT_DOUBLE_EQ(middle_band[6], 848);
  EXPECT_DOUBLE_EQ(middle_band[7], 860);
  EXPECT_DOUBLE_EQ(middle_band[8], 865);
  EXPECT_DOUBLE_EQ(middle_band[9], 862);

  EXPECT_TRUE(std::isnan(upper_band[0]));
  EXPECT_TRUE(std::isnan(upper_band[1]));
  EXPECT_TRUE(std::isnan(upper_band[2]));
  EXPECT_TRUE(std::isnan(upper_band[3]));
  EXPECT_DOUBLE_EQ(upper_band[4], 897.35341001239215);
  EXPECT_DOUBLE_EQ(upper_band[5], 897.35341001239215);
  EXPECT_DOUBLE_EQ(upper_band[6], 903.35341001239215);
  EXPECT_DOUBLE_EQ(upper_band[7], 887.5680975041804);
  EXPECT_DOUBLE_EQ(upper_band[8], 877.64911064067348);
  EXPECT_DOUBLE_EQ(upper_band[9], 875.56465996625059);

  EXPECT_TRUE(std::isnan(lower_band[0]));
  EXPECT_TRUE(std::isnan(lower_band[1]));
  EXPECT_TRUE(std::isnan(lower_band[2]));
  EXPECT_TRUE(std::isnan(lower_band[3]));
  EXPECT_DOUBLE_EQ(lower_band[4], 786.64658998760785);
  EXPECT_DOUBLE_EQ(lower_band[5], 786.64658998760785);
  EXPECT_DOUBLE_EQ(lower_band[6], 792.64658998760785);
  EXPECT_DOUBLE_EQ(lower_band[7], 832.4319024958196);
  EXPECT_DOUBLE_EQ(lower_band[8], 852.35088935932652);
  EXPECT_DOUBLE_EQ(lower_band[9], 848.43534003374941);
}
