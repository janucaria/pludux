#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(BooleanMethodTest, AndMethod)
{
  const auto true_condition = TrueMethod{};
  const auto false_condition = FalseMethod{};
  const auto asset_data = AssetHistory{};

  const auto true_and_true = AndMethod{true_condition, true_condition};
  EXPECT_TRUE(true_and_true(asset_data, context));

  const auto true_and_false = AndMethod{true_condition, false_condition};
  EXPECT_FALSE(true_and_false(asset_data, context));

  const auto false_and_true = AndMethod{false_condition, true_condition};
  EXPECT_FALSE(false_and_true(asset_data, context));

  const auto false_and_false = AndMethod{false_condition, false_condition};
  EXPECT_FALSE(false_and_false(asset_data, context));
}

TEST(BooleanMethodTest, EqualityAndMethod)
{
  const auto true_condition = TrueMethod{};
  const auto condition1 = AndMethod{true_condition, true_condition};
  const auto condition2 = AndMethod{true_condition, true_condition};

  EXPECT_TRUE(condition1 == condition2);
  EXPECT_FALSE(condition1 != condition2);
  EXPECT_EQ(condition1, condition2);
}

TEST(BooleanMethodTest, NotEqualAndMethod)
{
  const auto true_condition = TrueMethod{};
  const auto false_condition = FalseMethod{};
  const auto condition1 = AndMethod{true_condition, false_condition};
  const auto condition2 = AndMethod{false_condition, true_condition};

  EXPECT_TRUE(condition1 != condition2);
  EXPECT_FALSE(condition1 == condition2);
  EXPECT_NE(condition1, condition2);
}

TEST(BooleanMethodTest, OrMethod)
{
  const auto true_condition = TrueMethod{};
  const auto false_condition = FalseMethod{};
  const auto asset_data = AssetHistory{};

  const auto true_or_true = OrMethod{true_condition, true_condition};
  EXPECT_TRUE(true_or_true(asset_data, context));

  const auto true_or_false = OrMethod{true_condition, false_condition};
  EXPECT_TRUE(true_or_false(asset_data, context));

  const auto false_or_true = OrMethod{false_condition, true_condition};
  EXPECT_TRUE(false_or_true(asset_data, context));

  const auto false_or_false = OrMethod{false_condition, false_condition};
  EXPECT_FALSE(false_or_false(asset_data, context));
}

TEST(BooleanMethodTest, EqualityOrMethod)
{
  const auto true_condition = TrueMethod{};
  const auto condition1 = OrMethod{true_condition, true_condition};
  const auto condition2 = OrMethod{true_condition, true_condition};

  EXPECT_TRUE(condition1 == condition2);
  EXPECT_FALSE(condition1 != condition2);
  EXPECT_EQ(condition1, condition2);
}

TEST(BooleanMethodTest, NotEqualOrMethod)
{
  const auto true_condition = TrueMethod{};
  const auto false_condition = FalseMethod{};
  const auto condition1 = OrMethod{true_condition, false_condition};
  const auto condition2 = OrMethod{false_condition, true_condition};

  EXPECT_TRUE(condition1 != condition2);
  EXPECT_FALSE(condition1 == condition2);
  EXPECT_NE(condition1, condition2);
}

TEST(BooleanMethodTest, NotMethod)
{
  const auto true_condition = TrueMethod{};
  const auto false_condition = FalseMethod{};
  const auto asset_data = AssetHistory{};

  const auto not_true = NotMethod{true_condition};
  EXPECT_FALSE(not_true(asset_data, context));

  const auto not_false = NotMethod{false_condition};
  EXPECT_TRUE(not_false(asset_data, context));
}

TEST(BooleanMethodTest, EqualityNotMethod)
{
  const auto true_condition = TrueMethod{};
  const auto not_condition1 = NotMethod{true_condition};
  const auto not_condition2 = NotMethod{true_condition};

  EXPECT_TRUE(not_condition1 == not_condition2);
  EXPECT_FALSE(not_condition1 != not_condition2);
  EXPECT_EQ(not_condition1, not_condition2);
}

TEST(BooleanMethodTest, NotEqualNotMethod)
{
  const auto true_condition = TrueMethod{};
  const auto false_condition = FalseMethod{};
  const auto not_condition1 = NotMethod{true_condition};
  const auto not_condition2 = NotMethod{false_condition};

  EXPECT_TRUE(not_condition1 != not_condition2);
  EXPECT_FALSE(not_condition1 == not_condition2);
  EXPECT_NE(not_condition1, not_condition2);
}

TEST(BooleanMethodTest, XorMethod)
{
  const auto true_condition = TrueMethod{};
  const auto false_condition = FalseMethod{};
  const auto asset_data = AssetHistory{};

  const auto true_xor_true = XorMethod{true_condition, true_condition};
  EXPECT_FALSE(true_xor_true(asset_data, context));

  const auto true_xor_false = XorMethod{true_condition, false_condition};
  EXPECT_TRUE(true_xor_false(asset_data, context));

  const auto false_xor_true = XorMethod{false_condition, true_condition};
  EXPECT_TRUE(false_xor_true(asset_data, context));

  const auto false_xor_false = XorMethod{false_condition, false_condition};
  EXPECT_FALSE(false_xor_false(asset_data, context));
}

TEST(BooleanMethodTest, EqualityXorMethod)
{
  const auto true_condition = TrueMethod{};
  const auto condition1 = XorMethod{true_condition, true_condition};
  const auto condition2 = XorMethod{true_condition, true_condition};

  EXPECT_TRUE(condition1 == condition2);
  EXPECT_FALSE(condition1 != condition2);
  EXPECT_EQ(condition1, condition2);
}

TEST(BooleanMethodTest, NotEqualXorMethod)
{
  const auto true_condition = TrueMethod{};
  const auto false_condition = FalseMethod{};
  const auto condition1 = XorMethod{true_condition, false_condition};
  const auto condition2 = XorMethod{false_condition, true_condition};

  EXPECT_TRUE(condition1 != condition2);
  EXPECT_FALSE(condition1 == condition2);
  EXPECT_NE(condition1, condition2);
}