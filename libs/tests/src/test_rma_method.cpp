#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(RmaMethodTest, RunOneMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto period = 5;
  const auto rma_method = RmaMethod{period, field_method, 0};
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

  const auto result = rma_method.run_one(asset_data);
  EXPECT_DOUBLE_EQ(result, 854.33056000000022);
}

TEST(RmaMethodTest, RunAllMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto offset = 0;
  const auto period = 5;
  const auto rma_method = RmaMethod{period, field_method, 0};
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

  const auto result = rma_method.run_all(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], 854.33056000000022);
  EXPECT_DOUBLE_EQ(result[1], 854.16320000000019);
  EXPECT_DOUBLE_EQ(result[2], 852.70400000000018);
  EXPECT_DOUBLE_EQ(result[3], 850.88000000000011);
  EXPECT_DOUBLE_EQ(result[4], 848.60000000000002);
  EXPECT_DOUBLE_EQ(result[5], 842);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(RmaMethodTest, RunOneMethodWithOffset)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto offset = 1;
  const auto period = 5;
  const auto rma_method = RmaMethod{period, field_method, offset};
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

  const auto result = rma_method.run_one(asset_data);
  EXPECT_DOUBLE_EQ(result, 854.16320000000019);
}

TEST(RmaMethodTest, RunAllMethodWithOffset)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto offset = 2;
  const auto period = 5;
  const auto rma_method = RmaMethod{period, field_method, offset};
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

  const auto result = rma_method.run_all(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size() - offset);
  EXPECT_DOUBLE_EQ(result[0], 852.70400000000018);
  EXPECT_DOUBLE_EQ(result[1], 850.88000000000011);
  EXPECT_DOUBLE_EQ(result[2], 848.60000000000002);
  EXPECT_DOUBLE_EQ(result[3], 842);
  EXPECT_TRUE(std::isnan(result[4]));
  EXPECT_TRUE(std::isnan(result[5]));
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
}