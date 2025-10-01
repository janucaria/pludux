#include <gtest/gtest.h>

#include <cmath>

import pludux;

using namespace pludux::screener;

TEST(WmaMethodTest, ConstructorInitialization)
{
  {
    auto wma_method = WmaMethod{};

    EXPECT_EQ(wma_method.period(), 20);
    EXPECT_EQ(wma_method.source(), CloseMethod{});
  }
  {
    auto wma_method = WmaMethod{20};

    EXPECT_EQ(wma_method.period(), 20);
    EXPECT_EQ(wma_method.source(), CloseMethod{});
  }
  {
    const auto wma_method = WmaMethod{OpenMethod{}, 5};

    EXPECT_EQ(wma_method.period(), 5);
    EXPECT_EQ(wma_method.source(), OpenMethod{});
  }
}

TEST(WmaMethodTest, RunAllMethod)
{
  const auto wma_method = WmaMethod{CloseMethod{}, 5};
  const auto asset_data = pludux::AssetHistory{
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(wma_method(asset_snapshot[0], context), 859.33333333333337);
  EXPECT_DOUBLE_EQ(wma_method(asset_snapshot[1], context), 862.66666666666663);
  EXPECT_DOUBLE_EQ(wma_method(asset_snapshot[2], context), 862.66666666666663);
  EXPECT_DOUBLE_EQ(wma_method(asset_snapshot[3], context), 858.66666666666663);
  EXPECT_DOUBLE_EQ(wma_method(asset_snapshot[4], context), 852.66666666666663);
  EXPECT_DOUBLE_EQ(wma_method(asset_snapshot[5], context), 841.66666666666663);
  EXPECT_TRUE(std::isnan(wma_method(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(wma_method(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(wma_method(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(wma_method(asset_snapshot[9], context)));
}

TEST(WmaMethodTest, EqualityOperator)
{
  const auto wma_method1 = WmaMethod{CloseMethod{}, 5};
  const auto wma_method2 = WmaMethod{CloseMethod{}, 5};

  EXPECT_TRUE(wma_method1 == wma_method2);
  EXPECT_EQ(wma_method1, wma_method2);
}

TEST(WmaMethodTest, NotEqualOperator)
{
  const auto wma_method1 = WmaMethod{DataMethod{"close"}, 5};
  const auto wma_method2 = WmaMethod{DataMethod{"close"}, 10};
  const auto wma_method3 = WmaMethod{DataMethod{"open"}, 10};

  EXPECT_TRUE(wma_method1 != wma_method2);
  EXPECT_NE(wma_method1, wma_method2);
  EXPECT_TRUE(wma_method1 != wma_method3);
  EXPECT_NE(wma_method1, wma_method3);
  EXPECT_TRUE(wma_method2 != wma_method3);
  EXPECT_NE(wma_method2, wma_method3);
}