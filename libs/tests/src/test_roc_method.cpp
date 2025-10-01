#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux::screener;

TEST(RocMethodTest, ConstructorInitialization)
{
  {
    auto roc_method = RocMethod{};

    EXPECT_EQ(roc_method.period(), 14);
    EXPECT_EQ(roc_method.source(), CloseMethod{});
  }
  {
    auto roc_method = RocMethod{20};

    EXPECT_EQ(roc_method.period(), 20);
    EXPECT_EQ(roc_method.source(), CloseMethod{});
  }
  {
    const auto roc_method = RocMethod{OpenMethod{}, 5};

    EXPECT_EQ(roc_method.period(), 5);
    EXPECT_EQ(roc_method.source(), OpenMethod{});
  }
}

TEST(RocMethodTest, RunAllMethod)
{
  const auto roc_method = RocMethod{5};
  const auto asset_data = pludux::AssetHistory{
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(roc_method(asset_snapshot[0], context), -1.7241379310344827);
  EXPECT_DOUBLE_EQ(roc_method(asset_snapshot[1], context), 2.9940119760479043);
  EXPECT_DOUBLE_EQ(roc_method(asset_snapshot[2], context), 7.50);
  EXPECT_DOUBLE_EQ(roc_method(asset_snapshot[3], context), 3.6144578313253013);
  EXPECT_DOUBLE_EQ(roc_method(asset_snapshot[4], context), 0.00);
  EXPECT_TRUE(std::isnan(roc_method(asset_snapshot[5], context)));
  EXPECT_TRUE(std::isnan(roc_method(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(roc_method(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(roc_method(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(roc_method(asset_snapshot[9], context)));
}

TEST(RocMethodTest, EqualityOperator)
{
  const auto roc_method1 = RocMethod{};
  const auto roc_method2 = RocMethod{};

  EXPECT_TRUE(roc_method1 == roc_method2);
  EXPECT_EQ(roc_method1, roc_method2);
}

TEST(RocMethodTest, NotEqualOperator)
{
  const auto roc_method1 = RocMethod{DataMethod{"close"}, 5};
  const auto roc_method2 = RocMethod{DataMethod{"close"}, 10};
  const auto roc_method3 = RocMethod{DataMethod{"open"}, 10};

  EXPECT_TRUE(roc_method1 != roc_method2);
  EXPECT_NE(roc_method1, roc_method2);
  EXPECT_TRUE(roc_method1 != roc_method3);
  EXPECT_NE(roc_method1, roc_method3);
  EXPECT_TRUE(roc_method2 != roc_method3);
  EXPECT_NE(roc_method2, roc_method3);
}
