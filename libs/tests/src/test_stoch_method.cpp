import pludux.asset_history;
import pludux.asset_snapshot;
import pludux.screener;
import pludux.series;

#include <gtest/gtest.h>

using namespace pludux;
using namespace pludux::screener;

TEST(StochMethodTest, RunAllMethod)
{
  const auto high_method = DataMethod{"high"};
  const auto low_method = DataMethod{"low"};
  const auto close_method = DataMethod{"close"};
  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 3;
  const auto asset_data = pludux::AssetHistory{
   {"high", {865, 865, 875, 880, 875, 875, 840, 840, 875, 925}},
   {"low", {850, 850, 855, 845, 855, 820, 800, 800, 830, 815}},
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  auto stoch_method = StochMethod{StochOutput::k,
                                  high_method,
                                  low_method,
                                  close_method,
                                  k_period,
                                  k_smooth,
                                  d_period};
  auto stoch_series = StochSeries{stoch_method.output(),
                                  high_method(asset_data),
                                  low_method(asset_data),
                                  close_method(asset_data),
                                  k_period,
                                  k_smooth,
                                  d_period};

  const auto result_k = stoch_method(asset_data);
  const auto expected_k = stoch_series;

  stoch_method.output(StochOutput::d);
  stoch_series.output(stoch_method.output());

  const auto result_d = stoch_method(asset_data);
  const auto expected_d = stoch_series;

  ASSERT_EQ(result_k.size(), expected_k.size());
  ASSERT_EQ(result_d.size(), expected_d.size());

  EXPECT_DOUBLE_EQ(result_k[0], expected_k[0]);
  EXPECT_DOUBLE_EQ(result_k[1], expected_k[1]);
  EXPECT_DOUBLE_EQ(result_k[2], expected_k[2]);
  EXPECT_DOUBLE_EQ(result_k[3], expected_k[3]);
  EXPECT_TRUE(std::isnan(result_k[4]) && std::isnan(expected_k[4]));
  EXPECT_TRUE(std::isnan(result_k[5]) && std::isnan(expected_k[5]));
  EXPECT_TRUE(std::isnan(result_k[6]) && std::isnan(expected_k[6]));
  EXPECT_TRUE(std::isnan(result_k[7]) && std::isnan(expected_k[7]));
  EXPECT_TRUE(std::isnan(result_k[8]) && std::isnan(expected_k[8]));
  EXPECT_TRUE(std::isnan(result_k[9]) && std::isnan(expected_k[9]));

  EXPECT_DOUBLE_EQ(result_d[0], expected_d[0]);
  EXPECT_DOUBLE_EQ(result_d[1], expected_d[1]);
  EXPECT_TRUE(std::isnan(result_d[2]) && std::isnan(expected_d[2]));
  EXPECT_TRUE(std::isnan(result_d[3]) && std::isnan(expected_d[3]));
  EXPECT_TRUE(std::isnan(result_d[4]) && std::isnan(expected_d[4]));
  EXPECT_TRUE(std::isnan(result_d[5]) && std::isnan(expected_d[5]));
  EXPECT_TRUE(std::isnan(result_d[6]) && std::isnan(expected_d[6]));
  EXPECT_TRUE(std::isnan(result_d[7]) && std::isnan(expected_d[7]));
  EXPECT_TRUE(std::isnan(result_d[8]) && std::isnan(expected_d[8]));
  EXPECT_TRUE(std::isnan(result_d[9]) && std::isnan(expected_d[9]));
}

TEST(StochMethodTest, EqualityOperator)
{
  const auto high_method = DataMethod{"high"};
  const auto low_method = DataMethod{"low"};
  const auto close_method = DataMethod{"close"};
  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 3;

  StochMethod stoch_method1{StochOutput::k,
                            high_method,
                            low_method,
                            close_method,
                            k_period,
                            k_smooth,
                            d_period};
  StochMethod stoch_method2{StochOutput::k,
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

  StochMethod stoch_method1{StochOutput::k,
                            high_method,
                            low_method,
                            close_method,
                            k_period,
                            k_smooth,
                            d_period};
  StochMethod stoch_method2{StochOutput::d,
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
