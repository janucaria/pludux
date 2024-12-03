#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(EmaMethodTest, RunOneMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto period = 5;
  const auto ema_method = EmaMethod{period, field_method, 0};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 0, 0, 0, 875, 0),
                                    pludux::Quote(0, 0, 0, 0, 830, 0),
                                    pludux::Quote(0, 0, 0, 0, 800, 0),
                                    pludux::Quote(0, 0, 0, 0, 835, 0),
                                    pludux::Quote(0, 0, 0, 0, 870, 0),
                                    pludux::Quote(0, 0, 0, 0, 875, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 855, 0)});
  const auto asset_data = AssetDataProvider{asset};
  
  const auto result = ema_method.run_one(asset_data);
  EXPECT_DOUBLE_EQ(result, 856.95061728395069);
}

TEST(EmaMethodTest, RunAllMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto offset = 0;
  const auto period = 5;
  const auto ema_method = EmaMethod{period, field_method, 0};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 0, 0, 0, 875, 0),
                                    pludux::Quote(0, 0, 0, 0, 830, 0),
                                    pludux::Quote(0, 0, 0, 0, 800, 0),
                                    pludux::Quote(0, 0, 0, 0, 835, 0),
                                    pludux::Quote(0, 0, 0, 0, 870, 0),
                                    pludux::Quote(0, 0, 0, 0, 875, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 855, 0)});
  const auto asset_data = AssetDataProvider{asset};
  
  const auto series = ema_method.run_all(asset_data);

  ASSERT_EQ(series.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(series[0], 856.95061728395069);
  EXPECT_DOUBLE_EQ(series[1], 857.92592592592598);
  EXPECT_DOUBLE_EQ(series[2], 856.88888888888891);
  EXPECT_DOUBLE_EQ(series[3], 855.33333333333337);
  EXPECT_DOUBLE_EQ(series[4], 853);
  EXPECT_DOUBLE_EQ(series[5], 842);
  EXPECT_TRUE(std::isnan(series[6]));
  EXPECT_TRUE(std::isnan(series[7]));
  EXPECT_TRUE(std::isnan(series[8]));
  EXPECT_TRUE(std::isnan(series[9]));
}

TEST(EmaMethodTest, RunOneMethodWithOffset)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto offset = 1;
  const auto period = 5;
  const auto ema_method = EmaMethod{period, field_method, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 0, 0, 0, 875, 0),
                                    pludux::Quote(0, 0, 0, 0, 830, 0),
                                    pludux::Quote(0, 0, 0, 0, 800, 0),
                                    pludux::Quote(0, 0, 0, 0, 835, 0),
                                    pludux::Quote(0, 0, 0, 0, 870, 0),
                                    pludux::Quote(0, 0, 0, 0, 875, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 855, 0)});
  const auto asset_data = AssetDataProvider{asset};
  
  const auto result = ema_method.run_one(asset_data);
  EXPECT_DOUBLE_EQ(result, 857.92592592592598);
}

TEST(EmaMethodTest, RunAllMethodWithOffset)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto offset = 2;
  const auto period = 5;
  const auto ema_method = EmaMethod{period, field_method, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 0, 0, 0, 875, 0),
                                    pludux::Quote(0, 0, 0, 0, 830, 0),
                                    pludux::Quote(0, 0, 0, 0, 800, 0),
                                    pludux::Quote(0, 0, 0, 0, 835, 0),
                                    pludux::Quote(0, 0, 0, 0, 870, 0),
                                    pludux::Quote(0, 0, 0, 0, 875, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 860, 0),
                                    pludux::Quote(0, 0, 0, 0, 855, 0)});
  const auto asset_data = AssetDataProvider{asset};
  
  const auto series = ema_method.run_all(asset_data);

  ASSERT_EQ(series.size(), asset.quotes().size() - offset);
  EXPECT_DOUBLE_EQ(series[0], 856.88888888888891);
  EXPECT_DOUBLE_EQ(series[1], 855.33333333333337);
  EXPECT_DOUBLE_EQ(series[2], 853);
  EXPECT_DOUBLE_EQ(series[3], 842);
  EXPECT_TRUE(std::isnan(series[4]));
  EXPECT_TRUE(std::isnan(series[5]));
  EXPECT_TRUE(std::isnan(series[6]));
  EXPECT_TRUE(std::isnan(series[7]));
}