#include <gtest/gtest.h>

#include <cmath>

import pludux;

using namespace pludux::screener;

TEST(SmaMethodTest, ConstructorInitialization)
{
  {
    auto sma_method = SmaMethod{};

    EXPECT_EQ(sma_method.period(), 20);
    EXPECT_EQ(sma_method.source(), CloseMethod{});
  }
  {
    auto sma_method = SmaMethod{20};
    EXPECT_EQ(sma_method.period(), 20);
    EXPECT_EQ(sma_method.source(), CloseMethod{});
  }
  {
    const auto sma_method = SmaMethod{OpenMethod{}, 5};

    EXPECT_EQ(sma_method.period(), 5);
    EXPECT_EQ(sma_method.source(), OpenMethod{});
  }
}

TEST(SmaMethodTest, RunAllMethod)
{
  const auto sma_method = SmaMethod{CloseMethod{}, 5};
  const auto asset_data = pludux::AssetHistory{
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(sma_method(asset_snapshot[0], context), 862);
  EXPECT_DOUBLE_EQ(sma_method(asset_snapshot[1], context), 865);
  EXPECT_DOUBLE_EQ(sma_method(asset_snapshot[2], context), 860);
  EXPECT_DOUBLE_EQ(sma_method(asset_snapshot[3], context), 848);
  EXPECT_DOUBLE_EQ(sma_method(asset_snapshot[4], context), 842);
  EXPECT_DOUBLE_EQ(sma_method(asset_snapshot[5], context), 842);
  EXPECT_TRUE(std::isnan(sma_method(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(sma_method(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(sma_method(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(sma_method(asset_snapshot[9], context)));
}

TEST(SmaMethodTest, EqualityOperator)
{
  const auto sma_method1 = SmaMethod{CloseMethod{}, 5};
  const auto sma_method2 = SmaMethod{CloseMethod{}, 5};

  EXPECT_TRUE(sma_method1 == sma_method2);
  EXPECT_EQ(sma_method1, sma_method2);
}

TEST(SmaMethodTest, NotEqualOperator)
{
  const auto sma_method1 = SmaMethod{DataMethod{"close"}, 1};
  const auto sma_method2 = SmaMethod{DataMethod{"open"}, 1};
  const auto sma_method3 = SmaMethod{DataMethod{"close"}, 2};

  EXPECT_NE(sma_method1, sma_method2);
  EXPECT_NE(sma_method1, sma_method3);
  EXPECT_NE(sma_method2, sma_method3);
}
