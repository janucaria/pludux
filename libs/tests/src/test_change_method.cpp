#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux::screener;

TEST(ChangeMethodTest, ConstructorInitialization)
{
  {
    auto change_method = ChangeMethod{};

    EXPECT_EQ(change_method.source(), CloseMethod{});
  }
  {
    auto change_method = ChangeMethod{CloseMethod{}};

    EXPECT_EQ(change_method.source(), CloseMethod{});
  }
  {
    const auto change_method = ChangeMethod{DataMethod{"close"}};

    EXPECT_EQ(change_method.source(), DataMethod{"close"});
  }
}

TEST(ChangeMethodTest, RunAllMethod)
{
  const auto change_method = ChangeMethod{};
  const auto asset_data = pludux::AssetHistory{
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(change_method(asset_snapshot[0], context), -5);
  EXPECT_DOUBLE_EQ(change_method(asset_snapshot[1], context), 0);
  EXPECT_DOUBLE_EQ(change_method(asset_snapshot[2], context), 0);
  EXPECT_DOUBLE_EQ(change_method(asset_snapshot[3], context), -15);
  EXPECT_DOUBLE_EQ(change_method(asset_snapshot[4], context), 5);
  EXPECT_DOUBLE_EQ(change_method(asset_snapshot[5], context), 35);
  EXPECT_DOUBLE_EQ(change_method(asset_snapshot[6], context), 35);
  EXPECT_DOUBLE_EQ(change_method(asset_snapshot[7], context), -30);
  EXPECT_DOUBLE_EQ(change_method(asset_snapshot[8], context), -45);
  EXPECT_TRUE(std::isnan(change_method(asset_snapshot[9], context)));
}

TEST(ChangeMethodTest, EqualityOperator)
{
  const auto change_method1 = ChangeMethod{};
  const auto change_method2 = ChangeMethod{};

  EXPECT_TRUE(change_method1 == change_method2);
  EXPECT_EQ(change_method1, change_method2);
}

TEST(ChangeMethodTest, NotEqualOperator)
{
  const auto change_method1 = ChangeMethod{DataMethod{"open"}};
  const auto change_method2 = ChangeMethod{DataMethod{"close"}};

  EXPECT_TRUE(change_method1 != change_method2);
  EXPECT_NE(change_method1, change_method2);
}
