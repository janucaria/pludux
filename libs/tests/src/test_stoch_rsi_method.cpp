#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/data_method.hpp>
#include <pludux/screener/stoch_rsi_method.hpp>
#include <pludux/series.hpp>

using namespace pludux;
using namespace pludux::screener;

TEST(StochRsiMethodTest, RunAllMethod)
{
  const auto rsi_input_method = DataMethod{"close"};
  const auto rsi_period = 5;
  const auto k_period = 5;
  const auto k_smooth = 3;
  const auto d_period = 3;
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

  auto stoch_rsi_method = StochRsiMethod{
   StochOutput::k, rsi_input_method, rsi_period, k_period, k_smooth, d_period};
  auto stoch_rsi_series = StochRsiSeries{stoch_rsi_method.output(),
                                         rsi_input_method(asset_data),
                                         rsi_period,
                                         k_period,
                                         k_smooth,
                                         d_period};

  const auto result_k = stoch_rsi_method(asset_data);
  const auto expected_k = stoch_rsi_series;

  stoch_rsi_method.output(StochOutput::d);
  stoch_rsi_series.output(stoch_rsi_method.output());

  const auto result_d = stoch_rsi_method(asset_data);
  const auto expected_d = stoch_rsi_series;

  ASSERT_EQ(result_k.size(), expected_k.size());
  ASSERT_EQ(result_d.size(), expected_d.size());

  EXPECT_DOUBLE_EQ(result_k[0], expected_k[0]);
  EXPECT_DOUBLE_EQ(result_k[1], expected_k[1]);
  EXPECT_DOUBLE_EQ(result_k[2], expected_k[2]);
  EXPECT_DOUBLE_EQ(result_k[3], expected_k[3]);
  EXPECT_DOUBLE_EQ(result_k[4], expected_k[4]);
  EXPECT_DOUBLE_EQ(result_k[5], expected_k[5]);
  EXPECT_DOUBLE_EQ(result_k[6], expected_k[6]);
  EXPECT_TRUE(std::isnan(result_k[7]) && std::isnan(expected_k[7]));
  EXPECT_TRUE(std::isnan(result_k[8]) && std::isnan(expected_k[8]));
  EXPECT_TRUE(std::isnan(result_k[9]) && std::isnan(expected_k[9]));
  EXPECT_TRUE(std::isnan(result_k[10]) && std::isnan(expected_k[10]));
  EXPECT_TRUE(std::isnan(result_k[11]) && std::isnan(expected_k[11]));
  EXPECT_TRUE(std::isnan(result_k[12]) && std::isnan(expected_k[12]));
  EXPECT_TRUE(std::isnan(result_k[13]) && std::isnan(expected_k[13]));
  EXPECT_TRUE(std::isnan(result_k[14]) && std::isnan(expected_k[14]));

  EXPECT_DOUBLE_EQ(result_d[0], expected_d[0]);
  EXPECT_DOUBLE_EQ(result_d[1], expected_d[1]);
  EXPECT_DOUBLE_EQ(result_d[2], expected_d[2]);
  EXPECT_DOUBLE_EQ(result_d[3], expected_d[3]);
  EXPECT_DOUBLE_EQ(result_d[4], expected_d[4]);
  EXPECT_TRUE(std::isnan(result_d[5]) && std::isnan(expected_d[5]));
  EXPECT_TRUE(std::isnan(result_d[6]) && std::isnan(expected_d[6]));
  EXPECT_TRUE(std::isnan(result_d[7]) && std::isnan(expected_d[7]));
  EXPECT_TRUE(std::isnan(result_d[8]) && std::isnan(expected_d[8]));
  EXPECT_TRUE(std::isnan(result_d[9]) && std::isnan(expected_d[9]));
  EXPECT_TRUE(std::isnan(result_d[10]) && std::isnan(expected_d[10]));
  EXPECT_TRUE(std::isnan(result_d[11]) && std::isnan(expected_d[11]));
  EXPECT_TRUE(std::isnan(result_d[12]) && std::isnan(expected_d[12]));
  EXPECT_TRUE(std::isnan(result_d[13]) && std::isnan(expected_d[13]));
  EXPECT_TRUE(std::isnan(result_d[14]) && std::isnan(expected_d[14]));
}
