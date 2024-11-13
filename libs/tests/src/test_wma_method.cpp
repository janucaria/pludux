#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(WmaMethodTest, RunOneMethod)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto period = 5;
  const auto wma_method = WmaMethod{period, field_method, 0};
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

  const auto result = wma_method.run_one(asset);
  EXPECT_DOUBLE_EQ(result, 859.33333333333337);
}

TEST(WmaMethodTest, RunAllMethod)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto offset = 0;
  const auto period = 5;
  const auto wma_method = WmaMethod{period, field_method, 0};
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

  const auto result = wma_method.run_all(asset);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], 859.33333333333337);
  EXPECT_DOUBLE_EQ(result[1], 862.66666666666663);
  EXPECT_DOUBLE_EQ(result[2], 862.66666666666663);
  EXPECT_DOUBLE_EQ(result[3], 858.66666666666663);
  EXPECT_DOUBLE_EQ(result[4], 852.66666666666663);
  EXPECT_DOUBLE_EQ(result[5], 841.66666666666663);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(WmaMethodTest, RunOneMethodWithOffset)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto offset = 1;
  const auto period = 5;
  const auto wma_method = WmaMethod{period, field_method, offset};
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

  const auto result = wma_method.run_one(asset);
  EXPECT_DOUBLE_EQ(result, 862.66666666666663);
}

TEST(WmaMethodTest, RunAllMethodWithOffset)
{
  const auto field = "close";
  const auto field_method = FieldMethod{field, 0};
  const auto offset = 2;
  const auto period = 5;
  const auto wma_method = WmaMethod{period, field_method, offset};
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

  const auto result = wma_method.run_all(asset);

  ASSERT_EQ(result.size(), asset.quotes().size() - offset);
  EXPECT_DOUBLE_EQ(result[0], 862.66666666666663);
  EXPECT_DOUBLE_EQ(result[1], 858.66666666666663);
  EXPECT_DOUBLE_EQ(result[2], 852.66666666666663);
  EXPECT_DOUBLE_EQ(result[3], 841.66666666666663);
  EXPECT_TRUE(std::isnan(result[4]));
  EXPECT_TRUE(std::isnan(result[5]));
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
}