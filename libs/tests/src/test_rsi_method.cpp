#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux::screener;

TEST(RsiMethodTest, ConstructorInitialization)
{
  {
    auto rsi_method = RsiMethod{};

    EXPECT_EQ(rsi_method.period(), 14);
    EXPECT_EQ(rsi_method.source(), CloseMethod{});
  }
  {
    auto rsi_method = RsiMethod{20};

    EXPECT_EQ(rsi_method.period(), 20);
    EXPECT_EQ(rsi_method.source(), CloseMethod{});
  }
  {
    const auto rsi_method = RsiMethod{OpenMethod{}, 5};

    EXPECT_EQ(rsi_method.period(), 5);
    EXPECT_EQ(rsi_method.source(), OpenMethod{});
  }
}

TEST(RsiMethodTest, RunAllMethod)
{
  const auto rsi_method = RsiMethod{CloseMethod{}, 5};
  const auto asset_data = pludux::AssetHistory{
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(rsi_method(asset_snapshot[0], context), 41.446303291958991);
  EXPECT_DOUBLE_EQ(rsi_method(asset_snapshot[1], context), 44.444444444444443);
  EXPECT_DOUBLE_EQ(rsi_method(asset_snapshot[2], context), 44.44444444444445);
  EXPECT_DOUBLE_EQ(rsi_method(asset_snapshot[3], context), 44.444444444444443);
  EXPECT_DOUBLE_EQ(rsi_method(asset_snapshot[4], context), 50);
  EXPECT_TRUE(std::isnan(rsi_method(asset_snapshot[5], context)));
  EXPECT_TRUE(std::isnan(rsi_method(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(rsi_method(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(rsi_method(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(rsi_method(asset_snapshot[9], context)));
}

TEST(RsiMethodTest, EqualityOperator)
{
  const auto rsi_method1 = RsiMethod{CloseMethod{}, 5};
  const auto rsi_method2 = RsiMethod{CloseMethod{}, 5};

  EXPECT_TRUE(rsi_method1 == rsi_method2);
  EXPECT_EQ(rsi_method1, rsi_method2);
}

TEST(RsiMethodTest, NotEqualOperator)
{
  const auto rsi_method1 = RsiMethod{DataMethod{"close"}, 5};
  const auto rsi_method2 = RsiMethod{DataMethod{"close"}, 10};
  const auto rsi_method3 = RsiMethod{DataMethod{"open"}, 10};

  EXPECT_TRUE(rsi_method1 != rsi_method2);
  EXPECT_NE(rsi_method1, rsi_method2);
  EXPECT_TRUE(rsi_method1 != rsi_method3);
  EXPECT_NE(rsi_method1, rsi_method3);
  EXPECT_TRUE(rsi_method2 != rsi_method3);
  EXPECT_NE(rsi_method2, rsi_method3);
}
