#include <gtest/gtest.h>

#include <cmath>

import pludux;

using namespace pludux::screener;

TEST(RvolMethodTest, RunOneMethod)
{
  const auto data_method = DataMethod{"close"};
  const auto period = 5;
  const auto sma_method = RvolMethod{data_method, period};
  const auto asset_data = pludux::AssetHistory{
   {"close",
    {11.01, 8.59, 14.1, 30.52, 34.18, 55.34, 43.26, 65.39, 90.97, 585.54}}};

  const auto result = sma_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, 0.55945121951219512);
}

TEST(RvolMethodTest, RunAllMethod)
{
  const auto data_method = DataMethod{"close"};
  const auto period = 5;
  const auto sma_method = RvolMethod{data_method, period};
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

TEST(RvolMethodTest, EqualityOperator)
{
  const auto data_method1 = DataMethod{"close"};
  const auto rvol_method1 = RvolMethod{data_method1, 5};

  const auto data_method2 = DataMethod{"close"};
  const auto rvol_method2 = RvolMethod{data_method2, 5};

  EXPECT_TRUE(rvol_method1 == rvol_method2);
  EXPECT_FALSE(rvol_method1 != rvol_method2);
  EXPECT_EQ(rvol_method1, rvol_method2);
}

TEST(RvolMethodTest, NotEqualOperator)
{
  const auto data_method1 = DataMethod{"close"};
  const auto rvol_method1 = RvolMethod{data_method1, 5};

  const auto data_method2 = DataMethod{"open"};
  const auto rvol_method2 = RvolMethod{data_method2, 5};

  EXPECT_FALSE(rvol_method1 == rvol_method2);
  EXPECT_TRUE(rvol_method1 != rvol_method2);
}
