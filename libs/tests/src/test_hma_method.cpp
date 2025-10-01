#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux::screener;

TEST(HmaMethodTest, ConstructorInitialization)
{
  {
    auto hma_method = HmaMethod{};

    EXPECT_EQ(hma_method.period(), 20);
    EXPECT_EQ(hma_method.source(), CloseMethod{});
  }
  {
    auto hma_method = HmaMethod{20};
    EXPECT_EQ(hma_method.period(), 20);
    EXPECT_EQ(hma_method.source(), CloseMethod{});
  }
  {
    const auto hma_method = HmaMethod{OpenMethod{}, 5};

    EXPECT_EQ(hma_method.period(), 5);
    EXPECT_EQ(hma_method.source(), OpenMethod{});
  }
}

TEST(HmaMethodTest, RunAllMethod)
{
  const auto hma_method = HmaMethod{CloseMethod{}, 5};
  const auto asset_data = pludux::AssetHistory{
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(hma_method(asset_snapshot[0], context), 855.11111111111097);
  EXPECT_DOUBLE_EQ(hma_method(asset_snapshot[1], context), 857.33333333333337);
  EXPECT_DOUBLE_EQ(hma_method(asset_snapshot[2], context), 862);
  EXPECT_DOUBLE_EQ(hma_method(asset_snapshot[3], context), 878.88888888888903);
  EXPECT_DOUBLE_EQ(hma_method(asset_snapshot[4], context), 887.66666666666686);
  EXPECT_TRUE(std::isnan(hma_method(asset_snapshot[5], context)));
  EXPECT_TRUE(std::isnan(hma_method(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(hma_method(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(hma_method(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(hma_method(asset_snapshot[9], context)));
}

TEST(HmaMethodTest, EqualityOperator)
{
  const auto hma_method1 = HmaMethod{CloseMethod{}, 5};
  const auto hma_method2 = HmaMethod{CloseMethod{}, 5};

  EXPECT_TRUE(hma_method1 == hma_method2);
  EXPECT_EQ(hma_method1, hma_method2);
}

TEST(HmaMethodTest, NotEqualOperator)
{
  const auto hma_method1 = HmaMethod{DataMethod{"close"}, 5};
  const auto hma_method2 = HmaMethod{DataMethod{"close"}, 10};
  const auto hma_method3 = HmaMethod{DataMethod{"open"}, 10};

  EXPECT_TRUE(hma_method1 != hma_method2);
  EXPECT_NE(hma_method1, hma_method2);
  EXPECT_TRUE(hma_method1 != hma_method3);
  EXPECT_NE(hma_method1, hma_method3);
  EXPECT_TRUE(hma_method2 != hma_method3);
  EXPECT_NE(hma_method2, hma_method3);
}
