import pludux.asset_history;
import pludux.asset_snapshot;
import pludux.screener;
import pludux.series;

#include <gtest/gtest.h>

using namespace pludux::screener;

TEST(RocMethodTest, RunOneMethod)
{
  const auto field_method = DataMethod{"close", 0};
  const auto period = 5;
  const auto roc_method = RocMethod{period, field_method, 0};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = roc_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, -1.7241379310344827);
}

TEST(RocMethodTest, RunAllMethod)
{
  const auto field_method = DataMethod{"close", 0};
  const auto offset = 0;
  const auto period = 5;
  const auto roc_method = RocMethod{period, field_method, 0};
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

TEST(RocMethodTest, RunOneMethodWithOffset)
{
  const auto field_method = DataMethod{"close", 0};
  const auto offset = 1;
  const auto period = 5;
  const auto roc_method = RocMethod{period, field_method, offset};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = roc_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, 2.9940119760479043);
}

TEST(RocMethodTest, RunAllMethodWithOffset)
{
  const auto field_method = DataMethod{"close", 0};
  const auto offset = 2;
  const auto period = 5;
  const auto roc_method = RocMethod{period, field_method, offset};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = roc_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size() - offset);
  EXPECT_DOUBLE_EQ(result[0], 7.50);
  EXPECT_DOUBLE_EQ(result[1], 3.6144578313253013);
  EXPECT_DOUBLE_EQ(result[2], 0.00);
  EXPECT_TRUE(std::isnan(result[3]));
  EXPECT_TRUE(std::isnan(result[4]));
  EXPECT_TRUE(std::isnan(result[5]));
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
}

TEST(RocMethodTest, EqualityOperator)
{
  const auto field_method1 = DataMethod{"close", 0};
  const auto period1 = 5;
  const auto roc_method1 = RocMethod{period1, field_method1, 0};

  const auto field_method2 = DataMethod{"close", 0};
  const auto period2 = 5;
  const auto roc_method2 = RocMethod{period2, field_method2, 0};

  EXPECT_TRUE(roc_method1 == roc_method2);
  EXPECT_FALSE(roc_method1 != roc_method2);
  EXPECT_EQ(roc_method1, roc_method2);
}

TEST(RocMethodTest, NotEqualOperator)
{
  const auto field_method1 = DataMethod{"close", 0};
  const auto period1 = 5;
  const auto roc_method1 = RocMethod{period1, field_method1, 0};

  const auto field_method2 = DataMethod{"open", 0};
  const auto period2 = 10;
  const auto roc_method2 = RocMethod{period2, field_method2, 0};

  EXPECT_TRUE(roc_method1 != roc_method2);
  EXPECT_FALSE(roc_method1 == roc_method2);
}
