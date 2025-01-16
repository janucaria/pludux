#include <gtest/gtest.h>
#include <pludux/asset_data_provider.hpp>
#include <pludux/screener/arithmetic_method.hpp>
#include <pludux/screener/value_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;
using pludux::Quote;

TEST(ArithmeticMethodTest, MultiplyMethod)
{
  const auto operand1 = ValueMethod{2.0};
  const auto operand2 = ValueMethod{3.0};
  const auto multiply_method = MultiplyMethod{operand1, operand2};
  const auto asset = pludux::Asset{"", std::vector<Quote>(5)};
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto result = multiply_method(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], 6.0);
  EXPECT_DOUBLE_EQ(result[1], 6.0);
  EXPECT_DOUBLE_EQ(result[2], 6.0);
  EXPECT_DOUBLE_EQ(result[3], 6.0);
  EXPECT_DOUBLE_EQ(result[4], 6.0);
}

TEST(ArithmeticMethodTest, DivideMethod)
{
  const auto operand1 = ValueMethod{6.0};
  const auto operand2 = ValueMethod{3.0};
  const auto divide_method = DivideMethod{operand1, operand2};
  const auto asset = pludux::Asset{"", std::vector<Quote>(5)};
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto result = divide_method(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], 2.0);
  EXPECT_DOUBLE_EQ(result[1], 2.0);
  EXPECT_DOUBLE_EQ(result[2], 2.0);
  EXPECT_DOUBLE_EQ(result[3], 2.0);
  EXPECT_DOUBLE_EQ(result[4], 2.0);
}

TEST(ArithmeticMethodTest, AddMethod)
{
  const auto operand1 = ValueMethod{2.0};
  const auto operand2 = ValueMethod{3.0};
  const auto add_method = AddMethod{operand1, operand2};
  const auto asset = pludux::Asset{"", std::vector<Quote>(5)};
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto result = add_method(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], 5.0);
  EXPECT_DOUBLE_EQ(result[1], 5.0);
  EXPECT_DOUBLE_EQ(result[2], 5.0);
  EXPECT_DOUBLE_EQ(result[3], 5.0);
  EXPECT_DOUBLE_EQ(result[4], 5.0);
}

TEST(ArithmeticMethodTest, SubtractMethod)
{
  const auto operand1 = ValueMethod{5.0};
  const auto operand2 = ValueMethod{3.0};
  const auto subtract_method = SubtractMethod{operand1, operand2};
  const auto asset = pludux::Asset{"", std::vector<Quote>(5)};
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto result = subtract_method(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], 2.0);
  EXPECT_DOUBLE_EQ(result[1], 2.0);
  EXPECT_DOUBLE_EQ(result[2], 2.0);
  EXPECT_DOUBLE_EQ(result[3], 2.0);
  EXPECT_DOUBLE_EQ(result[4], 2.0);
}

TEST(ArithmeticMethodTest, NegateMethod)
{
  const auto operand = ValueMethod{5.0};
  const auto negate_method = NegateMethod{operand};
  const auto asset = pludux::Asset{"", std::vector<Quote>(5)};
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto result = negate_method(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], -5.0);
  EXPECT_DOUBLE_EQ(result[1], -5.0);
  EXPECT_DOUBLE_EQ(result[2], -5.0);
  EXPECT_DOUBLE_EQ(result[3], -5.0);
  EXPECT_DOUBLE_EQ(result[4], -5.0);
}