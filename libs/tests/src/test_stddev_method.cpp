#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(StddevMethodTest, ConstructorInitialization)
{
  {
    const auto stddev_method = StddevMethod{};
    EXPECT_EQ(stddev_method, StddevMethod{20});
    EXPECT_EQ(stddev_method.period(), 20);
    EXPECT_EQ(stddev_method.source(), CloseMethod{});
  }
  {
    const auto stddev_method = StddevMethod{15};
    EXPECT_EQ(stddev_method.period(), 15);
    EXPECT_EQ(stddev_method.source(), CloseMethod{});
  }
  {
    const auto source_method = CloseMethod{};
    const auto stddev_method = StddevMethod{source_method, 10};
    EXPECT_EQ(stddev_method.period(), 10);
    EXPECT_EQ(stddev_method.source(), CloseMethod{});
  }
}

TEST(StddevMethodTest, RunAllMethod)
{
  const auto asset_data =
   AssetHistory{{"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  const auto stddev_method = StddevMethod{5};

  EXPECT_DOUBLE_EQ(stddev_method(asset_snapshot[0], context),
                   6.7823299831252681);
  EXPECT_DOUBLE_EQ(stddev_method(asset_snapshot[1], context),
                   6.324555320336759);
  EXPECT_DOUBLE_EQ(stddev_method(asset_snapshot[2], context),
                   13.784048752090222);
  EXPECT_DOUBLE_EQ(stddev_method(asset_snapshot[3], context),
                   27.676705006196094);
  EXPECT_DOUBLE_EQ(stddev_method(asset_snapshot[4], context),
                   27.676705006196094);
  EXPECT_DOUBLE_EQ(stddev_method(asset_snapshot[5], context),
                   27.676705006196094);
  EXPECT_TRUE(std::isnan(stddev_method(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(stddev_method(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(stddev_method(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(stddev_method(asset_snapshot[9], context)));
}

TEST(StddevMethodTest, EqualityOperator)
{
  const auto stddev_method1 = StddevMethod{14};
  const auto stddev_method2 = StddevMethod{14};

  EXPECT_TRUE(stddev_method1 == stddev_method2);
  EXPECT_EQ(stddev_method1, stddev_method2);
}

TEST(StddevMethodTest, NotEqualOperator)
{
  const auto stddev_method1 = StddevMethod{14};
  const auto stddev_method2 = StddevMethod{20};
  const auto stddev_method3 = StddevMethod{CloseMethod{}, 10};

  EXPECT_TRUE(stddev_method1 != stddev_method2);
  EXPECT_NE(stddev_method1, stddev_method2);
  EXPECT_TRUE(stddev_method1 != stddev_method3);
  EXPECT_NE(stddev_method1, stddev_method3);
  EXPECT_TRUE(stddev_method2 != stddev_method3);
  EXPECT_NE(stddev_method2, stddev_method3);
}
