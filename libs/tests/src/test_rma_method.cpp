#include <gtest/gtest.h>

import pludux;

using namespace pludux::screener;

TEST(RmaMethodTest, RunOneMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field};
  const auto period = 5;
  const auto rma_method = RmaMethod{field_method, period};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = rma_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, 854.33056000000022);
}

TEST(RmaMethodTest, RunAllMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field};
  const auto period = 5;
  const auto rma_method = RmaMethod{field_method, period};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = rma_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
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

TEST(RmaMethodTest, EqualityOperator)
{
  const auto operand1_method1 = DataMethod{"close"};
  const auto rma_method1 = RmaMethod{operand1_method1, 5};

  const auto operand1_method2 = DataMethod{"close"};
  const auto rma_method2 = RmaMethod{operand1_method2, 5};

  EXPECT_TRUE(rma_method1 == rma_method2);
  EXPECT_FALSE(rma_method1 != rma_method2);
  EXPECT_EQ(rma_method1, rma_method2);
}

TEST(RmaMethodTest, NotEqualOperator)
{
  const auto operand1_method1 = DataMethod{"close"};
  const auto rma_method1 = RmaMethod{operand1_method1, 5};

  const auto operand1_method2 = DataMethod{"close"};
  const auto rma_method2 = RmaMethod{operand1_method2, 10};

  EXPECT_FALSE(rma_method1 == rma_method2);
  EXPECT_TRUE(rma_method1 != rma_method2);
  EXPECT_NE(rma_method1, rma_method2);
}