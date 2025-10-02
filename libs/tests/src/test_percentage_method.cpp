#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(PercentageMethodTest, ConstructorInitialization)
{
  {
    auto percentage_method = PercentageMethod{};
    EXPECT_EQ(percentage_method.percent(), 100.0);
    EXPECT_EQ(percentage_method.base(), CloseMethod{});
  }
  {
    auto percentage_method = PercentageMethod{20.0};
    EXPECT_EQ(percentage_method.percent(), 20.0);
    EXPECT_EQ(percentage_method.base(), CloseMethod{});
  }
  {
    const auto percentage_method = PercentageMethod{OpenMethod{}, 5.0};

    EXPECT_EQ(percentage_method.percent(), 5.0);
    EXPECT_EQ(percentage_method.base(), OpenMethod{});
  }
}

TEST(PercentageMethodTest, RunAllMethod)
{
  const auto percentage_method = PercentageMethod{CloseMethod{}, 5};
  const auto asset_data =
   AssetHistory{{"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(percentage_method(asset_snapshot[0], context), 42.75);
  EXPECT_DOUBLE_EQ(percentage_method(asset_snapshot[1], context), 43.0);
  EXPECT_DOUBLE_EQ(percentage_method(asset_snapshot[2], context), 43.0);
  EXPECT_DOUBLE_EQ(percentage_method(asset_snapshot[3], context), 43.0);
  EXPECT_DOUBLE_EQ(percentage_method(asset_snapshot[4], context), 43.75);
  EXPECT_DOUBLE_EQ(percentage_method(asset_snapshot[5], context), 43.5);
  EXPECT_DOUBLE_EQ(percentage_method(asset_snapshot[6], context), 41.75);
  EXPECT_DOUBLE_EQ(percentage_method(asset_snapshot[7], context), 40.0);
  EXPECT_DOUBLE_EQ(percentage_method(asset_snapshot[8], context), 41.5);
  EXPECT_DOUBLE_EQ(percentage_method(asset_snapshot[9], context), 43.75);
}

TEST(PercentageMethodTest, EqualityOperator)
{
  const auto percentage_method1 = PercentageMethod{CloseMethod{}, 5};
  const auto percentage_method2 = PercentageMethod{CloseMethod{}, 5};

  EXPECT_TRUE(percentage_method1 == percentage_method2);
  EXPECT_EQ(percentage_method1, percentage_method2);
}

TEST(PercentageMethodTest, NotEqualOperator)
{
  const auto percentage_method1 = PercentageMethod{DataMethod{"close"}, 5};
  const auto percentage_method2 = PercentageMethod{DataMethod{"close"}, 10};
  const auto percentage_method3 = PercentageMethod{DataMethod{"open"}, 10};

  EXPECT_TRUE(percentage_method1 != percentage_method2);
  EXPECT_NE(percentage_method1, percentage_method2);
  EXPECT_TRUE(percentage_method1 != percentage_method3);
  EXPECT_NE(percentage_method1, percentage_method3);
  EXPECT_TRUE(percentage_method2 != percentage_method3);
  EXPECT_NE(percentage_method2, percentage_method3);
}
