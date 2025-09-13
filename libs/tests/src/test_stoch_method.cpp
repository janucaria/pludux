#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/data_method.hpp>
#include <pludux/screener/stoch_method.hpp>
#include <pludux/series.hpp>

using namespace pludux;
using namespace pludux::screener;

TEST(StochMethodTest, RunAllMethod)
{
  const auto high_method = DataMethod{"high"};
  const auto low_method = DataMethod{"low"};
  const auto close_method = DataMethod{"close"};
  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 2;
  const auto asset_data = pludux::AssetHistory{
   {"high", {865, 865, 875, 880, 875, 875, 840, 840, 875, 925}},
   {"low", {850, 850, 855, 845, 855, 820, 800, 800, 830, 815}},
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  auto stoch_method = StochMethod{OutputName::StochasticK,
                                  high_method,
                                  low_method,
                                  close_method,
                                  k_period,
                                  k_smooth,
                                  d_period};

  const auto result_k = stoch_method(asset_data);

  stoch_method.output(OutputName::StochasticD);

  const auto result_d = stoch_method(asset_data);

  ASSERT_EQ(result_k.size(), 10);
  EXPECT_DOUBLE_EQ(result_k[0], 56.746031746031747);
  EXPECT_DOUBLE_EQ(result_k[1], 72.222222222222229);
  EXPECT_DOUBLE_EQ(result_k[2], 83.333333333333329);
  EXPECT_DOUBLE_EQ(result_k[3], 77);
  EXPECT_TRUE(std::isnan(result_k[4]));
  EXPECT_TRUE(std::isnan(result_k[5]));
  EXPECT_TRUE(std::isnan(result_k[6]));
  EXPECT_TRUE(std::isnan(result_k[7]));
  EXPECT_TRUE(std::isnan(result_k[8]));
  EXPECT_TRUE(std::isnan(result_k[9]));

  ASSERT_EQ(result_d.size(), 10);
  EXPECT_DOUBLE_EQ(result_d[0], 64.484126984126988);
  EXPECT_DOUBLE_EQ(result_d[1], 77.777777777777778);
  EXPECT_DOUBLE_EQ(result_d[2], 80.166666666666667);
  EXPECT_TRUE(std::isnan(result_d[3]));
  EXPECT_TRUE(std::isnan(result_d[4]));
  EXPECT_TRUE(std::isnan(result_d[5]));
  EXPECT_TRUE(std::isnan(result_d[6]));
  EXPECT_TRUE(std::isnan(result_d[7]));
  EXPECT_TRUE(std::isnan(result_d[8]));
  EXPECT_TRUE(std::isnan(result_d[9]));
}

TEST(StochMethodTest, EqualityOperator)
{
  const auto high_method = DataMethod{"high"};
  const auto low_method = DataMethod{"low"};
  const auto close_method = DataMethod{"close"};
  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 3;

  StochMethod stoch_method1{OutputName::StochasticK,
                            high_method,
                            low_method,
                            close_method,
                            k_period,
                            k_smooth,
                            d_period};
  StochMethod stoch_method2{OutputName::StochasticK,
                            high_method,
                            low_method,
                            close_method,
                            k_period,
                            k_smooth,
                            d_period};

  EXPECT_TRUE(stoch_method1 == stoch_method2);
  EXPECT_FALSE(stoch_method1 != stoch_method2);
  EXPECT_EQ(stoch_method1, stoch_method2);
}

TEST(StochMethodTest, NotEqualOperator)
{
  const auto high_method = DataMethod{"high"};
  const auto low_method = DataMethod{"low"};
  const auto close_method = DataMethod{"close"};
  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 3;

  StochMethod stoch_method1{OutputName::StochasticK,
                            high_method,
                            low_method,
                            close_method,
                            k_period,
                            k_smooth,
                            d_period};
  StochMethod stoch_method2{OutputName::StochasticD,
                            high_method,
                            low_method,
                            close_method,
                            k_period,
                            k_smooth,
                            d_period};

  EXPECT_TRUE(stoch_method1 != stoch_method2);
  EXPECT_FALSE(stoch_method1 == stoch_method2);
  EXPECT_NE(stoch_method1, stoch_method2);
}
