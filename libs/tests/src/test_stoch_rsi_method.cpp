#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(StochRsiMethodTest, RunAllMethod)
{
  const auto rsi_input_method = DataMethod{"close"};
  const auto rsi_period = 3;
  const auto k_period = 5;
  const auto k_smooth = 4;
  const auto d_period = 2;
  const auto asset_data = AssetHistory{{"close",
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

  const auto result_k =
   StochRsiMethod{rsi_input_method, rsi_period, k_period, k_smooth, d_period};
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[0], context), 0);
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[1], context), 25);
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[2], context), 25);
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[3], context), 25);
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[4], context), 37.18499098803867);
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[5], context), 24.369981976077337);
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[6], context), 49.36998197607734);
  EXPECT_DOUBLE_EQ(result_k(asset_snapshot[7], context), 74.369981976077341);
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[9], context)));
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[10], context)));
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[11], context)));
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[12], context)));
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[13], context)));
  EXPECT_TRUE(std::isnan(result_k(asset_snapshot[14], context)));

  const auto result_d = SelectOutputMethod{result_k, SeriesOutput::DPercent};
  EXPECT_DOUBLE_EQ(result_d(asset_snapshot[0], context), 12.5);
  EXPECT_DOUBLE_EQ(result_d(asset_snapshot[1], context), 25);
  EXPECT_DOUBLE_EQ(result_d(asset_snapshot[2], context), 25);
  EXPECT_DOUBLE_EQ(result_d(asset_snapshot[3], context), 31.092495494019335);
  EXPECT_DOUBLE_EQ(result_d(asset_snapshot[4], context), 30.777486482058002);
  EXPECT_DOUBLE_EQ(result_d(asset_snapshot[5], context), 36.869981976077341);
  EXPECT_DOUBLE_EQ(result_d(asset_snapshot[6], context), 61.869981976077341);
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[9], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[10], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[11], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[12], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[13], context)));
  EXPECT_TRUE(std::isnan(result_d(asset_snapshot[14], context)));
}

TEST(StochRsiMethodTest, EqualityOperator)
{
  const auto rsi_input_method = DataMethod{"close"};
  const auto rsi_period = 5;
  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 3;

  const auto method1 =
   StochRsiMethod{rsi_input_method, rsi_period, k_period, k_smooth, d_period};
  const auto method2 =
   StochRsiMethod{rsi_input_method, rsi_period, k_period, k_smooth, d_period};

  EXPECT_TRUE(method1 == method2);
  EXPECT_EQ(method1, method2);
}

TEST(StochRsiMethodTest, NotEqualOperator)
{
  const auto rsi_input_method = DataMethod{"close"};

  const auto method1 = StochRsiMethod{rsi_input_method, 5, 5, 3, 3};
  const auto method2 = StochRsiMethod{rsi_input_method, 5, 6, 2, 3};
  const auto method3 = StochRsiMethod{rsi_input_method, 5, 6, 2, 1};

  EXPECT_TRUE(method1 != method2);
  EXPECT_NE(method1, method2);
  EXPECT_TRUE(method1 != method3);
  EXPECT_NE(method1, method3);
  EXPECT_TRUE(method2 != method3);
  EXPECT_NE(method2, method3);
}