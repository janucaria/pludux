#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(RmaMethodTest, ConstructorInitialization)
{
  {
    auto rma_method = RmaMethod{};

    EXPECT_EQ(rma_method.period(), 20);
    EXPECT_EQ(rma_method.source(), CloseMethod{});
  }
  {
    auto rma_method = RmaMethod{20};
    EXPECT_EQ(rma_method.period(), 20);
    EXPECT_EQ(rma_method.source(), CloseMethod{});
  }
  {
    const auto rma_method = RmaMethod{OpenMethod{}, 5};

    EXPECT_EQ(rma_method.period(), 5);
    EXPECT_EQ(rma_method.source(), OpenMethod{});
  }
}

TEST(RmaMethodTest, RunAllMethod)
{
  const auto rma_method = RmaMethod{CloseMethod{}, 5};
  const auto asset_data = AssetHistory{
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(rma_method(asset_snapshot[0], context), 854.33056000000022);
  EXPECT_DOUBLE_EQ(rma_method(asset_snapshot[1], context), 854.16320000000019);
  EXPECT_DOUBLE_EQ(rma_method(asset_snapshot[2], context), 852.70400000000018);
  EXPECT_DOUBLE_EQ(rma_method(asset_snapshot[3], context), 850.88000000000011);
  EXPECT_DOUBLE_EQ(rma_method(asset_snapshot[4], context), 848.60000000000002);
  EXPECT_DOUBLE_EQ(rma_method(asset_snapshot[5], context), 842);
  EXPECT_TRUE(std::isnan(rma_method(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(rma_method(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(rma_method(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(rma_method(asset_snapshot[9], context)));
}

TEST(RmaMethodTest, EqualityOperator)
{
  const auto rma_method1 = RmaMethod{CloseMethod{}, 5};
  const auto rma_method2 = RmaMethod{CloseMethod{}, 5};

  EXPECT_TRUE(rma_method1 == rma_method2);
  EXPECT_EQ(rma_method1, rma_method2);
}

TEST(RmaMethodTest, NotEqualOperator)
{
  const auto rma_method1 = RmaMethod{DataMethod{"close"}, 5};
  const auto rma_method2 = RmaMethod{DataMethod{"close"}, 10};
  const auto rma_method3 = RmaMethod{DataMethod{"open"}, 10};

  EXPECT_TRUE(rma_method1 != rma_method2);
  EXPECT_NE(rma_method1, rma_method2);
  EXPECT_TRUE(rma_method2 != rma_method3);
  EXPECT_NE(rma_method2, rma_method3);
  EXPECT_TRUE(rma_method1 != rma_method3);
  EXPECT_NE(rma_method1, rma_method3);
}