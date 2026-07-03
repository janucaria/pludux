#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(LowestMethodTest, RunWithDynamicPeriodMethod)
{
  const auto period = AddMethod{ValueMethod{1.0}, ValueMethod{2.0}};
  const auto lowest_method = LowestMethod{CloseMethod{}, period};
  const auto asset_data = AssetHistory{{"Close", {1.0, 2.0, 3.0, 4.0, 5.0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lowest_method, asset_snapshot[0], context), 1.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lowest_method, asset_snapshot[1], context), 2.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lowest_method, asset_snapshot[2], context), 3.0);
  EXPECT_TRUE(std::isnan(
   evaluate_series_method(lowest_method, asset_snapshot[3], context)));
}

TEST(LowestMethodTest, EqualityOperator)
{
  const auto period = AddMethod{ValueMethod{1.0}, ValueMethod{2.0}};
  const auto lowest_method1 = LowestMethod{CloseMethod{}, period};
  const auto lowest_method2 = LowestMethod{CloseMethod{}, period};

  EXPECT_TRUE(lowest_method1 == lowest_method2);
  EXPECT_EQ(lowest_method1, lowest_method2);
}

TEST(LowestMethodTest, NotEqualOperator)
{
  const auto period1 = AddMethod{ValueMethod{1.0}, ValueMethod{2.0}};
  const auto period2 = AddMethod{ValueMethod{2.0}, ValueMethod{2.0}};
  const auto lowest_method1 = LowestMethod{CloseMethod{}, period1};
  const auto lowest_method2 = LowestMethod{CloseMethod{}, period2};

  EXPECT_TRUE(lowest_method1 != lowest_method2);
  EXPECT_NE(lowest_method1, lowest_method2);
}
