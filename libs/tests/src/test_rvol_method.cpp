import pludux.asset_history;
import pludux.asset_snapshot;
import pludux.screener;
import pludux.series;

#include <gtest/gtest.h>

using namespace pludux::screener;

TEST(RvolMethodTest, RunOneMethod)
{
  const auto data_method = DataMethod{"close", 0};
  const auto period = 5;
  const auto sma_method = RvolMethod{period, data_method, 0};
  const auto asset_data = pludux::AssetHistory{
   {"close",
    {11.01, 8.59, 14.1, 30.52, 34.18, 55.34, 43.26, 65.39, 90.97, 585.54}}};

  const auto result = sma_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, 0.55945121951219512);
}

TEST(RvolMethodTest, RunAllMethod)
{
  const auto data_method = DataMethod{"close", 0};
  const auto offset = 0;
  const auto period = 5;
  const auto sma_method = RvolMethod{period, data_method, 0};
  const auto asset_data = pludux::AssetHistory{
   {"close",
    {11.01, 8.59, 14.1, 30.52, 34.18, 55.34, 43.26, 65.39, 90.97, 585.54}}};

  const auto result = sma_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(result[0], 0.55945121951219512);
  EXPECT_DOUBLE_EQ(result[1], 0.30091781685700275);
  EXPECT_DOUBLE_EQ(result[2], 0.39740698985343859);
  EXPECT_DOUBLE_EQ(result[3], 0.66727884909703095);
  EXPECT_DOUBLE_EQ(result[4], 0.59106315279795263);
  EXPECT_DOUBLE_EQ(result[5], 0.32920880428316479);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(RvolMethodTest, RunOneMethodWithOffset)
{
  const auto data_method = DataMethod{"close", 0};
  const auto offset = 1;
  const auto period = 5;
  const auto sma_method = RvolMethod{period, data_method, offset};
  const auto asset_data = pludux::AssetHistory{
   {"close",
    {11.01, 8.59, 14.1, 30.52, 34.18, 55.34, 43.26, 65.39, 90.97, 585.54}}};

  const auto result = sma_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, 0.30091781685700275);
}

TEST(RvolMethodTest, RunAllMethodWithOffset)
{
  const auto data_method = DataMethod{"close", 0};
  const auto offset = 2;
  const auto period = 5;
  const auto sma_method = RvolMethod{period, data_method, offset};
  const auto asset_data = pludux::AssetHistory{
   {"close",
    {11.01, 8.59, 14.1, 30.52, 34.18, 55.34, 43.26, 65.39, 90.97, 585.54}}};

  const auto result = sma_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size() - offset);
  EXPECT_DOUBLE_EQ(result[0], 0.39740698985343859);
  EXPECT_DOUBLE_EQ(result[1], 0.66727884909703095);
  EXPECT_DOUBLE_EQ(result[2], 0.59106315279795263);
  EXPECT_DOUBLE_EQ(result[3], 0.32920880428316479);
  EXPECT_TRUE(std::isnan(result[4]));
  EXPECT_TRUE(std::isnan(result[5]));
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
}

TEST(RvolMethodTest, EqualityOperator)
{
  const auto data_method1 = DataMethod{"close", 0};
  const auto rvol_method1 = RvolMethod{5, data_method1, 0};

  const auto data_method2 = DataMethod{"close", 0};
  const auto rvol_method2 = RvolMethod{5, data_method2, 0};

  EXPECT_TRUE(rvol_method1 == rvol_method2);
  EXPECT_FALSE(rvol_method1 != rvol_method2);
  EXPECT_EQ(rvol_method1, rvol_method2);
}

TEST(RvolMethodTest, NotEqualOperator)
{
  const auto data_method1 = DataMethod{"close", 0};
  const auto rvol_method1 = RvolMethod{5, data_method1, 0};

  const auto data_method2 = DataMethod{"open", 0};
  const auto rvol_method2 = RvolMethod{5, data_method2, 0};

  EXPECT_FALSE(rvol_method1 == rvol_method2);
  EXPECT_TRUE(rvol_method1 != rvol_method2);
}
