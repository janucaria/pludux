#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/arithmetic_method.hpp>
#include <pludux/screener/data_method.hpp>
#include <pludux/screener/value_method.hpp>
#include <pludux/series.hpp>

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