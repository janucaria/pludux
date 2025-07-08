import pludux.series;

#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener.hpp>

using namespace pludux::screener;

TEST(WmaMethodTest, RunOneMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto period = 5;
  const auto wma_method = WmaMethod{period, field_method, 0};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = wma_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, 859.33333333333337);
}

TEST(WmaMethodTest, RunAllMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto offset = 0;
  const auto period = 5;
  const auto wma_method = WmaMethod{period, field_method, 0};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = wma_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
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
  const auto field_method = DataMethod{field, 0};
  const auto offset = 1;
  const auto period = 5;
  const auto wma_method = WmaMethod{period, field_method, offset};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = wma_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, 862.66666666666663);
}

TEST(WmaMethodTest, RunAllMethodWithOffset)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto offset = 2;
  const auto period = 5;
  const auto wma_method = WmaMethod{period, field_method, offset};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = wma_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size() - offset);
  EXPECT_DOUBLE_EQ(result[0], 862.66666666666663);
  EXPECT_DOUBLE_EQ(result[1], 858.66666666666663);
  EXPECT_DOUBLE_EQ(result[2], 852.66666666666663);
  EXPECT_DOUBLE_EQ(result[3], 841.66666666666663);
  EXPECT_TRUE(std::isnan(result[4]));
  EXPECT_TRUE(std::isnan(result[5]));
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
}

TEST(WmaMethodTest, EqualityOperator)
{
  const auto field = "close";
  const auto field_method = DataMethod{field, 0};
  const auto period = 5;
  const auto wma_method1 = WmaMethod{period, field_method, 0};
  const auto wma_method2 = WmaMethod{period, field_method, 0};

  EXPECT_TRUE(wma_method1 == wma_method2);
  EXPECT_FALSE(wma_method1 != wma_method2);
  EXPECT_EQ(wma_method1, wma_method2);
}

TEST(WmaMethodTest, NotEqualOperator)
{
  const auto field1 = "close";
  const auto field_method1 = DataMethod{field1, 0};
  const auto period1 = 5;
  const auto wma_method1 = WmaMethod{period1, field_method1, 0};

  const auto field2 = "open";
  const auto field_method2 = DataMethod{field2, 0};
  const auto period2 = 10;
  const auto wma_method2 = WmaMethod{period2, field_method2, 0};

  EXPECT_TRUE(wma_method1 != wma_method2);
  EXPECT_FALSE(wma_method1 == wma_method2);
  EXPECT_NE(wma_method1, wma_method2);
}