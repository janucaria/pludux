#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(StochMethodTest, RunAllMethod)
{
  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 2;
  const auto asset_data = AssetHistory{
   {"High", {865, 865, 875, 880, 875, 875, 840, 840, 875, 925}},
   {"Low", {850, 850, 855, 845, 855, 820, 800, 800, 830, 815}},
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  const auto result_k = StochMethod{k_period, k_smooth, d_period};
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[0], context), 56.746031746031747);
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[1], context), 72.222222222222229);
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[2], context), 83.333333333333329);
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[3], context), 77);
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[4], context)));
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[5], context)));
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[9], context)));

  const auto result_d = SelectOutputMethod{result_k, SeriesOutput::DPercent};
  EXPECT_DOUBLE_EQ(result_d(asset_snapshot[0], context), 64.484126984126988);
  EXPECT_DOUBLE_EQ(result_d(asset_snapshot[1], context), 77.777777777777778);
  EXPECT_DOUBLE_EQ(result_d(asset_snapshot[2], context), 80.166666666666667);
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[3], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[4], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[5], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[9], context)));
}

TEST(StochMethodTest, EqualityOperator)
{
  const auto stoch_method1 = StochMethod{5, 3, 3};
  const auto stoch_method2 = StochMethod{5, 3, 3};

  EXPECT_TRUE(stoch_method1 == stoch_method2);
  EXPECT_EQ(stoch_method1, stoch_method2);
}

TEST(StochMethodTest, NotEqualOperator)
{
  const auto stoch_method1 = StochMethod{5, 3, 3};
  const auto stoch_method2 = StochMethod{5, 3, 8};
  const auto stoch_method3 = StochMethod{4, 3, 8};

  EXPECT_TRUE(stoch_method1 != stoch_method2);
  EXPECT_NE(stoch_method1, stoch_method2);
  EXPECT_TRUE(stoch_method1 != stoch_method3);
  EXPECT_NE(stoch_method1, stoch_method3);
  EXPECT_TRUE(stoch_method2 != stoch_method3);
  EXPECT_NE(stoch_method2, stoch_method3);
}
