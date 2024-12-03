#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(HmaMethodTest, RunOneMethod)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto period = 5;
  const auto hma_method = HmaMethod{period, field_method, 0};
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
  
  const auto result = hma_method.run_one(asset_data);
  EXPECT_DOUBLE_EQ(result, 855.11111111111097);
}

TEST(HmaMethodTest, RunAllMethod)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto offset = 0;
  const auto period = 5;
  const auto hma_method = HmaMethod{period, field_method, 0};
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
  
  const auto result = hma_method.run_all(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
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

TEST(HmaMethodTest, RunOneMethodWithOffset)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto offset = 1;
  const auto period = 5;
  const auto hma_method = HmaMethod{period, field_method, offset};
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
  
  const auto result = hma_method.run_one(asset_data);
  EXPECT_DOUBLE_EQ(result, 857.33333333333337);
}

TEST(HmaMethodTest, RunAllMethodWithOffset)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto offset = 2;
  const auto period = 5;
  const auto hma_method = HmaMethod{period, field_method, offset};
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
  
  const auto result = hma_method.run_all(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size() - offset);
  EXPECT_DOUBLE_EQ(result[0], 862);
  EXPECT_DOUBLE_EQ(result[1], 878.88888888888903);
  EXPECT_DOUBLE_EQ(result[2], 887.66666666666686);
  EXPECT_TRUE(std::isnan(result[3]));
  EXPECT_TRUE(std::isnan(result[4]));
  EXPECT_TRUE(std::isnan(result[5]));
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
}