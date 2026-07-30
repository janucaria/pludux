#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(HighestMethodTest, RunWithDynamicPeriodMethod)
{
  const auto period = AddMethod{ValueMethod{1.0}, ValueMethod{2.0}};
  const auto highest_method = HighestMethod{CloseMethod{}, period};
  const auto asset_data = AssetHistory{{"Close", {1.0, 2.0, 3.0, 4.0, 5.0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(highest_method, asset_snapshot[0], context), 5.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(highest_method, asset_snapshot[1], context), 4.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(highest_method, asset_snapshot[2], context), 3.0);
  EXPECT_TRUE(std::isnan(
   evaluate_series_method(highest_method, asset_snapshot[3], context)));
}

TEST(HighestMethodTest, EqualityOperator)
{
  const auto period = AddMethod{ValueMethod{1.0}, ValueMethod{2.0}};
  const auto highest_method1 = HighestMethod{CloseMethod{}, period};
  const auto highest_method2 = HighestMethod{CloseMethod{}, period};

  EXPECT_TRUE(highest_method1 == highest_method2);
  EXPECT_EQ(highest_method1, highest_method2);
}

TEST(HighestMethodTest, NotEqualOperator)
{
  const auto period1 = AddMethod{ValueMethod{1.0}, ValueMethod{2.0}};
  const auto period2 = AddMethod{ValueMethod{2.0}, ValueMethod{2.0}};
  const auto highest_method1 = HighestMethod{CloseMethod{}, period1};
  const auto highest_method2 = HighestMethod{CloseMethod{}, period2};

  EXPECT_TRUE(highest_method1 != highest_method2);
  EXPECT_NE(highest_method1, highest_method2);
}
