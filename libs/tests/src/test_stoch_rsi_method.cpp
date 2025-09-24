#include <gtest/gtest.h>

#include <cmath>

import pludux;

using namespace pludux;
using namespace pludux::screener;

TEST(StochRsiMethodTest, RunAllMethod)
{
  const auto rsi_input_method = DataMethod{"close"};
  const auto rsi_period = 3;
  const auto k_period = 5;
  const auto k_smooth = 4;
  const auto d_period = 2;
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

  auto stoch_rsi_method =
   StochRsiMethod{rsi_input_method, rsi_period, k_period, k_smooth, d_period};

  const auto result_k = stoch_rsi_method(asset_data);
  const auto result_d =
   OutputByNameMethod{stoch_rsi_method, OutputName::StochasticD}(asset_data);

  ASSERT_EQ(result_k.size(), 15);
  EXPECT_DOUBLE_EQ(result_k[0], 0);
  EXPECT_DOUBLE_EQ(result_k[1], 25);
  EXPECT_DOUBLE_EQ(result_k[2], 25);
  EXPECT_DOUBLE_EQ(result_k[3], 25);
  EXPECT_DOUBLE_EQ(result_k[4], 37.18499098803867);
  EXPECT_DOUBLE_EQ(result_k[5], 24.369981976077337);
  EXPECT_DOUBLE_EQ(result_k[6], 49.36998197607734);
  EXPECT_DOUBLE_EQ(result_k[7], 74.369981976077341);
  EXPECT_TRUE(std::isnan(result_k[8]));
  EXPECT_TRUE(std::isnan(result_k[9]));
  EXPECT_TRUE(std::isnan(result_k[10]));
  EXPECT_TRUE(std::isnan(result_k[11]));
  EXPECT_TRUE(std::isnan(result_k[12]));
  EXPECT_TRUE(std::isnan(result_k[13]));
  EXPECT_TRUE(std::isnan(result_k[14]));

  ASSERT_EQ(result_d.size(), 15);
  EXPECT_DOUBLE_EQ(result_d[0], 12.5);
  EXPECT_DOUBLE_EQ(result_d[1], 25);
  EXPECT_DOUBLE_EQ(result_d[2], 25);
  EXPECT_DOUBLE_EQ(result_d[3], 31.092495494019335);
  EXPECT_DOUBLE_EQ(result_d[4], 30.777486482058002);
  EXPECT_DOUBLE_EQ(result_d[5], 36.869981976077341);
  EXPECT_DOUBLE_EQ(result_d[6], 61.869981976077341);
  EXPECT_TRUE(std::isnan(result_d[7]));
  EXPECT_TRUE(std::isnan(result_d[8]));
  EXPECT_TRUE(std::isnan(result_d[9]));
  EXPECT_TRUE(std::isnan(result_d[10]));
  EXPECT_TRUE(std::isnan(result_d[11]));
  EXPECT_TRUE(std::isnan(result_d[12]));
  EXPECT_TRUE(std::isnan(result_d[13]));
  EXPECT_TRUE(std::isnan(result_d[14]));
}

TEST(StochRsiMethodTest, EqualityOperator)
{
  const auto rsi_input_method = DataMethod{"close"};
  const auto rsi_period = 5;
  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 3;

  StochRsiMethod method1{
   rsi_input_method, rsi_period, k_period, k_smooth, d_period};
  StochRsiMethod method2{
   rsi_input_method, rsi_period, k_period, k_smooth, d_period};

  EXPECT_TRUE(method1 == method2);
  EXPECT_FALSE(method1 != method2);
  EXPECT_EQ(method1, method2);
}

TEST(StochRsiMethodTest, NotEqualOperator)
{
  const auto rsi_input_method = DataMethod{"close"};

  StochRsiMethod method1{rsi_input_method, 5, 5, 3, 3};
  StochRsiMethod method2{rsi_input_method, 5, 6, 2, 3};

  EXPECT_FALSE(method1 == method2);
  EXPECT_TRUE(method1 != method2);
  EXPECT_NE(method1, method2);
}