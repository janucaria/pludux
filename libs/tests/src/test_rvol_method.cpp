#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux::screener;

TEST(RvolMethodTest, ConstructorInitialization)
{
  {
    auto rvol_method = RvolMethod{};
    EXPECT_EQ(rvol_method.period(), 14);
  }
  {
    auto rvol_method = RvolMethod{20};
    EXPECT_EQ(rvol_method.period(), 20);
  }
}

TEST(RvolMethodTest, RunAllMethod)
{
  const auto rvol_method = RvolMethod{5};
  const auto asset_data = pludux::AssetHistory{
   {"Volume",
    {11.01, 8.59, 14.1, 30.52, 34.18, 55.34, 43.26, 65.39, 90.97, 585.54}}};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(rvol_method(asset_snapshot[0], context),
                   0.55945121951219512);
  EXPECT_DOUBLE_EQ(rvol_method(asset_snapshot[1], context),
                   0.30091781685700275);
  EXPECT_DOUBLE_EQ(rvol_method(asset_snapshot[2], context),
                   0.39740698985343859);
  EXPECT_DOUBLE_EQ(rvol_method(asset_snapshot[3], context),
                   0.66727884909703095);
  EXPECT_DOUBLE_EQ(rvol_method(asset_snapshot[4], context),
                   0.59106315279795263);
  EXPECT_DOUBLE_EQ(rvol_method(asset_snapshot[5], context),
                   0.32920880428316479);
  EXPECT_TRUE(std::isnan(rvol_method(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(rvol_method(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(rvol_method(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(rvol_method(asset_snapshot[9], context)));
}

TEST(RvolMethodTest, EqualityOperator)
{
  const auto rvol_method1 = RvolMethod{5};
  const auto rvol_method2 = RvolMethod{5};

  EXPECT_TRUE(rvol_method1 == rvol_method2);
  EXPECT_EQ(rvol_method1, rvol_method2);
}

TEST(RvolMethodTest, NotEqualOperator)
{
  const auto rvol_method1 = RvolMethod{5};
  const auto rvol_method2 = RvolMethod{10};

  EXPECT_TRUE(rvol_method1 != rvol_method2);
  EXPECT_NE(rvol_method1, rvol_method2);
}
