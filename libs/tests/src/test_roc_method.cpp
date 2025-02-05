#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

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