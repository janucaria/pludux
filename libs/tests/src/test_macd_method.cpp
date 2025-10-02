#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(MacdMethodTest, ConstructorInitialization)
{
  {
    auto macd_method = MacdMethod{};

    EXPECT_EQ(macd_method.fast_period(), 12);
    EXPECT_EQ(macd_method.slow_period(), 26);
    EXPECT_EQ(macd_method.signal_period(), 9);
    EXPECT_EQ(macd_method.source(), CloseMethod{});
  }
  {
    auto macd_method = MacdMethod{5, 10, 3};

    EXPECT_EQ(macd_method.fast_period(), 5);
    EXPECT_EQ(macd_method.slow_period(), 10);
    EXPECT_EQ(macd_method.signal_period(), 3);
    EXPECT_EQ(macd_method.source(), CloseMethod{});
  }
  {
    const auto macd_method = MacdMethod{OpenMethod{}, 5, 10, 3};

    EXPECT_EQ(macd_method.fast_period(), 5);
    EXPECT_EQ(macd_method.slow_period(), 10);
    EXPECT_EQ(macd_method.signal_period(), 3);
    EXPECT_EQ(macd_method.source(), OpenMethod{});
  }
}

TEST(MacdMethodTest, RunAllMethod)
{
  const auto fast_period = 5;
  const auto slow_period = 10;
  const auto signal_period = 3;
  const auto asset_data = AssetHistory{{"Close",
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
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  auto macd_line =
   MacdMethod{CloseMethod{}, fast_period, slow_period, signal_period};

  EXPECT_DOUBLE_EQ(macd_line(asset_snapshot[0], context), -12.656409004137572);
  EXPECT_DOUBLE_EQ(macd_line(asset_snapshot[1], context), -7.6929729699789959);
  EXPECT_DOUBLE_EQ(macd_line(asset_snapshot[2], context), -3.2941210218015158);
  EXPECT_DOUBLE_EQ(macd_line(asset_snapshot[3], context), 0.969787663390548);
  EXPECT_DOUBLE_EQ(macd_line(asset_snapshot[4], context), 4.5125327347551547);
  EXPECT_DOUBLE_EQ(macd_line(asset_snapshot[5], context), 4.9506172839506917);
  EXPECT_TRUE(std::isnan(macd_line(asset_snapshot[10], context)));
  EXPECT_TRUE(std::isnan(macd_line(asset_snapshot[11], context)));
  EXPECT_TRUE(std::isnan(macd_line(asset_snapshot[12], context)));
  EXPECT_TRUE(std::isnan(macd_line(asset_snapshot[13], context)));
  EXPECT_TRUE(std::isnan(macd_line(asset_snapshot[14], context)));
  EXPECT_TRUE(std::isnan(macd_line(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(macd_line(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(macd_line(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(macd_line(asset_snapshot[9], context)));

  const auto signal_line =
   SelectOutputMethod{macd_line, MethodOutput::SignalLine};
  EXPECT_DOUBLE_EQ(signal_line(asset_snapshot[0], context),
                   -8.2285071355347075);
  EXPECT_DOUBLE_EQ(signal_line(asset_snapshot[1], context),
                   -3.8006052669318442);
  EXPECT_DOUBLE_EQ(signal_line(asset_snapshot[2], context),
                   0.091762436115307766);
  EXPECT_DOUBLE_EQ(signal_line(asset_snapshot[3], context), 3.4776458940321313);
  EXPECT_TRUE(std::isnan(signal_line(asset_snapshot[10], context)));
  EXPECT_TRUE(std::isnan(signal_line(asset_snapshot[11], context)));
  EXPECT_TRUE(std::isnan(signal_line(asset_snapshot[12], context)));
  EXPECT_TRUE(std::isnan(signal_line(asset_snapshot[13], context)));
  EXPECT_TRUE(std::isnan(signal_line(asset_snapshot[14], context)));
  EXPECT_TRUE(std::isnan(signal_line(asset_snapshot[4], context)));
  EXPECT_TRUE(std::isnan(signal_line(asset_snapshot[5], context)));
  EXPECT_TRUE(std::isnan(signal_line(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(signal_line(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(signal_line(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(signal_line(asset_snapshot[9], context)));

  const auto histograms =
   SelectOutputMethod{macd_line, MethodOutput::Histogram};
  EXPECT_DOUBLE_EQ(histograms(asset_snapshot[0], context), -4.4279018686028646);
  EXPECT_DOUBLE_EQ(histograms(asset_snapshot[1], context), -3.8923677030471517);
  EXPECT_DOUBLE_EQ(histograms(asset_snapshot[2], context), -3.3858834579168233);
  EXPECT_DOUBLE_EQ(histograms(asset_snapshot[3], context), -2.5078582306415833);
  EXPECT_TRUE(std::isnan(histograms(asset_snapshot[10], context)));
  EXPECT_TRUE(std::isnan(histograms(asset_snapshot[11], context)));
  EXPECT_TRUE(std::isnan(histograms(asset_snapshot[12], context)));
  EXPECT_TRUE(std::isnan(histograms(asset_snapshot[13], context)));
  EXPECT_TRUE(std::isnan(histograms(asset_snapshot[14], context)));
  EXPECT_TRUE(std::isnan(histograms(asset_snapshot[4], context)));
  EXPECT_TRUE(std::isnan(histograms(asset_snapshot[5], context)));
  EXPECT_TRUE(std::isnan(histograms(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(histograms(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(histograms(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(histograms(asset_snapshot[9], context)));
}

TEST(MacdMethodTest, EqualityOperator)
{
  const auto macd_method1 = MacdMethod{CloseMethod{}, 5, 10, 3};
  const auto macd_method2 = MacdMethod{CloseMethod{}, 5, 10, 3};

  EXPECT_TRUE(macd_method1 == macd_method2);
  EXPECT_EQ(macd_method1, macd_method2);
}

TEST(MacdMethodTest, NotEqualOperator)
{
  const auto macd_method1 = MacdMethod{DataMethod{"close"}, 5, 10, 3};
  const auto macd_method2 = MacdMethod{DataMethod{"open"}, 5, 10, 3};
  const auto macd_method3 = MacdMethod{DataMethod{"open"}, 5, 10, 5};

  EXPECT_TRUE(macd_method1 != macd_method2);
  EXPECT_NE(macd_method1, macd_method2);
  EXPECT_TRUE(macd_method1 != macd_method3);
  EXPECT_NE(macd_method1, macd_method3);
  EXPECT_TRUE(macd_method2 != macd_method3);
  EXPECT_NE(macd_method2, macd_method3);
}
