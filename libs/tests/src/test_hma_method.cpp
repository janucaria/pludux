#include <gtest/gtest.h>

#include <cmath>

import pludux;

using namespace pludux::screener;

TEST(HmaMethodTest, RunOneMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field};
  const auto period = 5;
  const auto hma_method = HmaMethod{field_method, period};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = hma_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, 855.11111111111097);
}

TEST(HmaMethodTest, RunAllMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field};
  const auto period = 5;
  const auto hma_method = HmaMethod{field_method, period};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = hma_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
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

TEST(HmaMethodTest, EqualityOperator)
{
  const auto field = "close";
  const auto field_method1 = DataMethod{field};
  const auto field_method2 = DataMethod{field};
  const auto period = 5;
  const auto hma_method1 = HmaMethod{field_method1, period};
  const auto hma_method2 = HmaMethod{field_method2, period};

  EXPECT_TRUE(hma_method1 == hma_method2);
  EXPECT_FALSE(hma_method1 != hma_method2);
  EXPECT_EQ(hma_method1, hma_method2);
}

TEST(HmaMethodTest, NotEqualOperator)
{
  const auto field1 = "close";
  const auto field2 = "open";
  const auto field_method1 = DataMethod{field1};
  const auto field_method2 = DataMethod{field2};
  const auto period = 5;
  const auto hma_method1 = HmaMethod{field_method1, period};
  const auto hma_method2 = HmaMethod{field_method2, period};

  EXPECT_TRUE(hma_method1 != hma_method2);
  EXPECT_FALSE(hma_method1 == hma_method2);
  EXPECT_NE(hma_method1, hma_method2);
}
