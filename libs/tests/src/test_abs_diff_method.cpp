#include <gtest/gtest.h>

#include <variant>

import pludux;

using namespace pludux;

TEST(AbsDiffMethodTest, RunMethod)
{
  const auto operand1_method = ValueMethod{20.0};
  const auto operand2_method = DataMethod{"close"};
  const auto abs_diff_method = AbsDiffMethod{operand1_method, operand2_method};
  const auto asset_data = AssetHistory{{"close", {10, 15, 20, 25, 30}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_DOUBLE_EQ(abs_diff_method(asset_snapshot[0], context), 10);
  EXPECT_DOUBLE_EQ(abs_diff_method(asset_snapshot[1], context), 5);
  EXPECT_DOUBLE_EQ(abs_diff_method(asset_snapshot[2], context), 0);
  EXPECT_DOUBLE_EQ(abs_diff_method(asset_snapshot[3], context), 5);
  EXPECT_DOUBLE_EQ(abs_diff_method(asset_snapshot[4], context), 10);
}

TEST(AbsDiffMethodTest, EqualityOperator)
{
  const auto operand1_method1 = ValueMethod{10.0};
  const auto operand2_method1 = DataMethod{"open"};
  const auto abs_diff_method1 =
   AbsDiffMethod{operand1_method1, operand2_method1};

  const auto operand1_method2 = ValueMethod{10.0};
  const auto operand2_method2 = DataMethod{"open"};
  const auto abs_diff_method2 =
   AbsDiffMethod{operand1_method2, operand2_method2};

  EXPECT_TRUE(abs_diff_method1 == abs_diff_method2);
  EXPECT_FALSE(abs_diff_method1 != abs_diff_method2);
  EXPECT_EQ(abs_diff_method1, abs_diff_method2);
}

TEST(AbsDiffMethodTest, NotEqualOperator)
{
  const auto operand1_method1 = ValueMethod{10.0};
  const auto operand2_method1 = DataMethod{"open"};
  const auto abs_diff_method1 =
   AbsDiffMethod{operand1_method1, operand2_method1};

  const auto operand1_method2 = ValueMethod{20.0};
  const auto operand2_method2 = DataMethod{"close"};
  const auto abs_diff_method2 =
   AbsDiffMethod{operand1_method2, operand2_method2};

  EXPECT_TRUE(abs_diff_method1 != abs_diff_method2);
  EXPECT_FALSE(abs_diff_method1 == abs_diff_method2);
  EXPECT_NE(abs_diff_method1, abs_diff_method2);
}
