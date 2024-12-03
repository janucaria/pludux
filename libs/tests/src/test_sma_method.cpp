#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(SmaMethodTest, RunOneMethod)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto period = 5;
  const auto sma_method = SmaMethod{period, field_method, 0};
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

  const auto result = sma_method.run_one(asset_data);
  EXPECT_DOUBLE_EQ(result, 862);
}

TEST(SmaMethodTest, RunAllMethod)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto offset = 0;
  const auto period = 5;
  const auto sma_method = SmaMethod{period, field_method, 0};
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

  const auto result = sma_method.run_all(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], 862);
  EXPECT_DOUBLE_EQ(result[1], 865);
  EXPECT_DOUBLE_EQ(result[2], 860);
  EXPECT_DOUBLE_EQ(result[3], 848);
  EXPECT_DOUBLE_EQ(result[4], 842);
  EXPECT_DOUBLE_EQ(result[5], 842);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(SmaMethodTest, RunOneMethodWithOffset)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto offset = 1;
  const auto period = 5;
  const auto sma_method = SmaMethod{period, field_method, offset};
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

  const auto result = sma_method.run_one(asset_data);
  EXPECT_DOUBLE_EQ(result, 865);
}

TEST(SmaMethodTest, RunAllMethodWithOffset)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto offset = 2;
  const auto period = 5;
  const auto sma_method = SmaMethod{period, field_method, offset};
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

  const auto result = sma_method.run_all(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size() - offset);
  EXPECT_DOUBLE_EQ(result[0], 860);
  EXPECT_DOUBLE_EQ(result[1], 848);
  EXPECT_DOUBLE_EQ(result[2], 842);
  EXPECT_DOUBLE_EQ(result[3], 842);
  EXPECT_TRUE(std::isnan(result[4]));
  EXPECT_TRUE(std::isnan(result[5]));
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
}