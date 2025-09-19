#include <gtest/gtest.h>

import pludux;

using namespace pludux::screener;

TEST(RocMethodTest, RunOneMethod)
{
  const auto field_method = DataMethod{"close"};
  const auto period = 5;
  const auto roc_method = RocMethod{field_method, period};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = roc_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, -1.7241379310344827);
}

TEST(RocMethodTest, RunAllMethod)
{
  const auto field_method = DataMethod{"close"};
  const auto period = 5;
  const auto roc_method = RocMethod{field_method, period};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = roc_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(result[0], -1.7241379310344827);
  EXPECT_DOUBLE_EQ(result[1], 2.9940119760479043);
  EXPECT_DOUBLE_EQ(result[2], 7.50);
  EXPECT_DOUBLE_EQ(result[3], 3.6144578313253013);
  EXPECT_DOUBLE_EQ(result[4], 0.00);
  EXPECT_TRUE(std::isnan(result[5]));
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(RocMethodTest, EqualityOperator)
{
  const auto field_method1 = DataMethod{"close"};
  const auto period1 = 5;
  const auto roc_method1 = RocMethod{field_method1, period1};

  const auto field_method2 = DataMethod{"close"};
  const auto period2 = 5;
  const auto roc_method2 = RocMethod{field_method2, period2};

  EXPECT_TRUE(roc_method1 == roc_method2);
  EXPECT_FALSE(roc_method1 != roc_method2);
  EXPECT_EQ(roc_method1, roc_method2);
}

TEST(RocMethodTest, NotEqualOperator)
{
  const auto field_method1 = DataMethod{"close"};
  const auto period1 = 5;
  const auto roc_method1 = RocMethod{field_method1, period1};

  const auto field_method2 = DataMethod{"open"};
  const auto period2 = 10;
  const auto roc_method2 = RocMethod{field_method2, period2};

  EXPECT_TRUE(roc_method1 != roc_method2);
  EXPECT_FALSE(roc_method1 == roc_method2);
}
