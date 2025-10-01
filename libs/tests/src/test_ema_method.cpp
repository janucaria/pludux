#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux::screener;

TEST(EmaMethodTest, ConstructorInitialization)
{
  {
    auto ema_method = EmaMethod{};

    EXPECT_EQ(ema_method.period(), 20);
    EXPECT_EQ(ema_method.source(), CloseMethod{});
  }
  {
    auto ema_method = EmaMethod{20};

    EXPECT_EQ(ema_method.period(), 20);
    EXPECT_EQ(ema_method.source(), CloseMethod{});
  }
  {
    const auto ema_method = EmaMethod{OpenMethod{}, 5};

    EXPECT_EQ(ema_method.period(), 5);
    EXPECT_EQ(ema_method.source(), OpenMethod{});
  }
}

TEST(EmaMethodTest, RunAllMethod)
{
  const auto ema_method = EmaMethod{CloseMethod{}, 5};
  const auto asset_data = pludux::AssetHistory{
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(ema_method(asset_snapshot[0], context), 856.95061728395069);
  EXPECT_DOUBLE_EQ(ema_method(asset_snapshot[1], context), 857.92592592592598);
  EXPECT_DOUBLE_EQ(ema_method(asset_snapshot[2], context), 856.88888888888891);
  EXPECT_DOUBLE_EQ(ema_method(asset_snapshot[3], context), 855.33333333333337);
  EXPECT_DOUBLE_EQ(ema_method(asset_snapshot[4], context), 853);
  EXPECT_DOUBLE_EQ(ema_method(asset_snapshot[5], context), 842);
  EXPECT_TRUE(std::isnan(ema_method(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(ema_method(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(ema_method(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(ema_method(asset_snapshot[9], context)));
}

TEST(EmaMethodTest, EqualityOperator)
{
  const auto ema_method1 = EmaMethod{CloseMethod{}, 5};
  const auto ema_method2 = EmaMethod{CloseMethod{}, 5};

  EXPECT_TRUE(ema_method1 == ema_method2);
  EXPECT_EQ(ema_method1, ema_method2);
}

TEST(EmaMethodTest, NotEqualOperator)
{
  const auto ema_method1 = EmaMethod{DataMethod{"close"}, 5};
  const auto ema_method2 = EmaMethod{DataMethod{"close"}, 10};
  const auto ema_method3 = EmaMethod{DataMethod{"open"}, 10};

  EXPECT_TRUE(ema_method1 != ema_method2);
  EXPECT_NE(ema_method1, ema_method2);
  EXPECT_TRUE(ema_method1 != ema_method3);
  EXPECT_NE(ema_method1, ema_method3);
  EXPECT_TRUE(ema_method2 != ema_method3);
  EXPECT_NE(ema_method2, ema_method3);
}
