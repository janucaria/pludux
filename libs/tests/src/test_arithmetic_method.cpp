#include <gtest/gtest.h>

import pludux;

using namespace pludux::screener;
using pludux::AssetSnapshot;

TEST(ArithmeticMethodTest, MultiplyMethod)
{
  const auto operand1 = ValueMethod{2.0};
  const auto operand2 = DataMethod{"close"};
  const auto multiply_method = MultiplyMethod{operand1, operand2};
  const auto asset_data = pludux::AssetHistory{{"close", {10, 15, 20, 25, 30}}};

  const auto result = multiply_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(result[0], 20.0);
  EXPECT_DOUBLE_EQ(result[1], 30.0);
  EXPECT_DOUBLE_EQ(result[2], 40.0);
  EXPECT_DOUBLE_EQ(result[3], 50.0);
  EXPECT_DOUBLE_EQ(result[4], 60.0);
}

TEST(ArithmeticMethodTest, MultiplyEqualityOperator)
{
  const auto operand1_method1 = ValueMethod{10.0};
  const auto operand2_method1 = DataMethod{"open"};
  const auto multiply_method1 =
   MultiplyMethod{operand1_method1, operand2_method1};

  const auto operand1_method2 = ValueMethod{10.0};
  const auto operand2_method2 = DataMethod{"open"};
  const auto multiply_method2 =
   MultiplyMethod{operand1_method2, operand2_method2};

  EXPECT_TRUE(multiply_method1 == multiply_method2);
  EXPECT_FALSE(multiply_method1 != multiply_method2);
  EXPECT_EQ(multiply_method1, multiply_method2);
}

TEST(ArithmeticMethodTest, MultiplyNotEqualOperator)
{
  const auto operand1_method1 = ValueMethod{10.0};
  const auto operand2_method1 = DataMethod{"open"};
  const auto multiply_method1 =
   MultiplyMethod{operand1_method1, operand2_method1};

  const auto operand1_method2 = ValueMethod{20.0};
  const auto operand2_method2 = DataMethod{"close"};
  const auto multiply_method2 =
   MultiplyMethod{operand1_method2, operand2_method2};

  EXPECT_TRUE(multiply_method1 != multiply_method2);
  EXPECT_FALSE(multiply_method1 == multiply_method2);
  EXPECT_NE(multiply_method1, multiply_method2);
}

TEST(ArithmeticMethodTest, DivideMethod)
{
  const auto operand1 = ValueMethod{20.0};
  const auto operand2 = DataMethod{"close"};
  const auto divide_method = DivideMethod{operand1, operand2};
  const auto asset_data = pludux::AssetHistory{{"close", {10, 15, 20, 25, 30}}};

  const auto result = divide_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(result[0], 2.0);
  EXPECT_DOUBLE_EQ(result[1], 1.3333333333333333);
  EXPECT_DOUBLE_EQ(result[2], 1.0);
  EXPECT_DOUBLE_EQ(result[3], 0.8);
  EXPECT_DOUBLE_EQ(result[4], 0.6666666666666666);
}

TEST(ArithmeticMethodTest, DivideEqualityOperator)
{
  const auto operand1_method1 = ValueMethod{10.0};
  const auto operand2_method1 = DataMethod{"open"};
  const auto divide_method1 = DivideMethod{operand1_method1, operand2_method1};

  const auto operand1_method2 = ValueMethod{10.0};
  const auto operand2_method2 = DataMethod{"open"};
  const auto divide_method2 = DivideMethod{operand1_method2, operand2_method2};

  EXPECT_TRUE(divide_method1 == divide_method2);
  EXPECT_FALSE(divide_method1 != divide_method2);
  EXPECT_EQ(divide_method1, divide_method2);
}

TEST(ArithmeticMethodTest, DivideNotEqualOperator)
{
  const auto operand1_method1 = ValueMethod{10.0};
  const auto operand2_method1 = DataMethod{"open"};
  const auto divide_method1 = DivideMethod{operand1_method1, operand2_method1};

  const auto operand1_method2 = ValueMethod{20.0};
  const auto operand2_method2 = DataMethod{"close"};
  const auto divide_method2 = DivideMethod{operand1_method2, operand2_method2};

  EXPECT_TRUE(divide_method1 != divide_method2);
  EXPECT_FALSE(divide_method1 == divide_method2);
  EXPECT_NE(divide_method1, divide_method2);
}

TEST(ArithmeticMethodTest, AddMethod)
{
  const auto operand1 = ValueMethod{2.0};
  const auto operand2 = DataMethod{"close"};
  const auto add_method = AddMethod{operand1, operand2};
  const auto asset_data = pludux::AssetHistory{{"close", {10, 15, 20, 25, 30}}};

  const auto result = add_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(result[0], 12.0);
  EXPECT_DOUBLE_EQ(result[1], 17.0);
  EXPECT_DOUBLE_EQ(result[2], 22.0);
  EXPECT_DOUBLE_EQ(result[3], 27.0);
  EXPECT_DOUBLE_EQ(result[4], 32.0);
}

TEST(ArithmeticMethodTest, AddEqualityOperator)
{
  const auto operand1_method1 = ValueMethod{10.0};
  const auto operand2_method1 = DataMethod{"open"};
  const auto add_method1 = AddMethod{operand1_method1, operand2_method1};

  const auto operand1_method2 = ValueMethod{10.0};
  const auto operand2_method2 = DataMethod{"open"};
  const auto add_method2 = AddMethod{operand1_method2, operand2_method2};

  EXPECT_TRUE(add_method1 == add_method2);
  EXPECT_FALSE(add_method1 != add_method2);
  EXPECT_EQ(add_method1, add_method2);
}

TEST(ArithmeticMethodTest, AddNotEqualOperator)
{
  const auto operand1_method1 = ValueMethod{10.0};
  const auto operand2_method1 = DataMethod{"open"};
  const auto add_method1 = AddMethod{operand1_method1, operand2_method1};

  const auto operand1_method2 = ValueMethod{20.0};
  const auto operand2_method2 = DataMethod{"close"};
  const auto add_method2 = AddMethod{operand1_method2, operand2_method2};

  EXPECT_TRUE(add_method1 != add_method2);
  EXPECT_FALSE(add_method1 == add_method2);
  EXPECT_NE(add_method1, add_method2);
}

TEST(ArithmeticMethodTest, SubtractMethod)
{
  const auto operand1 = ValueMethod{15.0};
  const auto operand2 = DataMethod{"close"};
  const auto subtract_method = SubtractMethod{operand1, operand2};
  const auto asset_data = pludux::AssetHistory{{"close", {10, 15, 20, 25, 30}}};

  const auto result = subtract_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(result[0], 5.0);
  EXPECT_DOUBLE_EQ(result[1], 0.0);
  EXPECT_DOUBLE_EQ(result[2], -5.0);
  EXPECT_DOUBLE_EQ(result[3], -10.0);
  EXPECT_DOUBLE_EQ(result[4], -15.0);
}

TEST(ArithmeticMethodTest, SubtractEqualityOperator)
{
  const auto operand1_method1 = ValueMethod{10.0};
  const auto operand2_method1 = DataMethod{"open"};
  const auto subtract_method1 =
   SubtractMethod{operand1_method1, operand2_method1};

  const auto operand1_method2 = ValueMethod{10.0};
  const auto operand2_method2 = DataMethod{"open"};
  const auto subtract_method2 =
   SubtractMethod{operand1_method2, operand2_method2};

  EXPECT_TRUE(subtract_method1 == subtract_method2);
  EXPECT_FALSE(subtract_method1 != subtract_method2);
  EXPECT_EQ(subtract_method1, subtract_method2);
}

TEST(ArithmeticMethodTest, SubtractNotEqualOperator)
{
  const auto operand1_method1 = ValueMethod{10.0};
  const auto operand2_method1 = DataMethod{"open"};
  const auto subtract_method1 =
   SubtractMethod{operand1_method1, operand2_method1};

  const auto operand1_method2 = ValueMethod{20.0};
  const auto operand2_method2 = DataMethod{"close"};
  const auto subtract_method2 =
   SubtractMethod{operand1_method2, operand2_method2};

  EXPECT_TRUE(subtract_method1 != subtract_method2);
  EXPECT_FALSE(subtract_method1 == subtract_method2);
  EXPECT_NE(subtract_method1, subtract_method2);
}

TEST(ArithmeticMethodTest, NegateMethod)
{
  const auto operand = DataMethod{"close"};
  const auto negate_method = NegateMethod{operand};
  const auto asset_data = pludux::AssetHistory{{"close", {10, 15, 20, 25, 30}}};

  const auto result = negate_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(result[0], -10.0);
  EXPECT_DOUBLE_EQ(result[1], -15.0);
  EXPECT_DOUBLE_EQ(result[2], -20.0);
  EXPECT_DOUBLE_EQ(result[3], -25.0);
  EXPECT_DOUBLE_EQ(result[4], -30.0);
}

TEST(ArithmeticMethodTest, NegateEqualityOperator)
{
  const auto operand_method1 = DataMethod{"open"};
  const auto negate_method1 = NegateMethod{operand_method1};

  const auto operand_method2 = DataMethod{"open"};
  const auto negate_method2 = NegateMethod{operand_method2};

  EXPECT_TRUE(negate_method1 == negate_method2);
  EXPECT_FALSE(negate_method1 != negate_method2);
  EXPECT_EQ(negate_method1, negate_method2);
}

TEST(ArithmeticMethodTest, NegateNotEqualOperator)
{
  const auto operand_method1 = DataMethod{"open"};
  const auto negate_method1 = NegateMethod{operand_method1};

  const auto operand_method2 = DataMethod{"close"};
  const auto negate_method2 = NegateMethod{operand_method2};

  EXPECT_TRUE(negate_method1 != negate_method2);
  EXPECT_FALSE(negate_method1 == negate_method2);
  EXPECT_NE(negate_method1, negate_method2);
}