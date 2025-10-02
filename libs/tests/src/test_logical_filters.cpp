#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(BooleanMethodTest, AndMethod)
{
  const auto true_filter = TrueMethod{};
  const auto false_filter = FalseMethod{};
  const auto asset_data = AssetHistory{};

  const auto true_and_true = AndMethod{true_filter, true_filter};
  EXPECT_TRUE(true_and_true(asset_data, context));

  const auto true_and_false = AndMethod{true_filter, false_filter};
  EXPECT_FALSE(true_and_false(asset_data, context));

  const auto false_and_true = AndMethod{false_filter, true_filter};
  EXPECT_FALSE(false_and_true(asset_data, context));

  const auto false_and_false = AndMethod{false_filter, false_filter};
  EXPECT_FALSE(false_and_false(asset_data, context));
}

TEST(BooleanMethodTest, EqualityAndMethod)
{
  const auto true_filter = TrueMethod{};
  const auto filter1 = AndMethod{true_filter, true_filter};
  const auto filter2 = AndMethod{true_filter, true_filter};

  EXPECT_TRUE(filter1 == filter2);
  EXPECT_FALSE(filter1 != filter2);
  EXPECT_EQ(filter1, filter2);
}

TEST(BooleanMethodTest, NotEqualAndMethod)
{
  const auto true_filter = TrueMethod{};
  const auto false_filter = FalseMethod{};
  const auto filter1 = AndMethod{true_filter, false_filter};
  const auto filter2 = AndMethod{false_filter, true_filter};

  EXPECT_TRUE(filter1 != filter2);
  EXPECT_FALSE(filter1 == filter2);
  EXPECT_NE(filter1, filter2);
}

TEST(BooleanMethodTest, OrMethod)
{
  const auto true_filter = TrueMethod{};
  const auto false_filter = FalseMethod{};
  const auto asset_data = AssetHistory{};

  const auto true_or_true = OrMethod{true_filter, true_filter};
  EXPECT_TRUE(true_or_true(asset_data, context));

  const auto true_or_false = OrMethod{true_filter, false_filter};
  EXPECT_TRUE(true_or_false(asset_data, context));

  const auto false_or_true = OrMethod{false_filter, true_filter};
  EXPECT_TRUE(false_or_true(asset_data, context));

  const auto false_or_false = OrMethod{false_filter, false_filter};
  EXPECT_FALSE(false_or_false(asset_data, context));
}

TEST(BooleanMethodTest, EqualityOrMethod)
{
  const auto true_filter = TrueMethod{};
  const auto filter1 = OrMethod{true_filter, true_filter};
  const auto filter2 = OrMethod{true_filter, true_filter};

  EXPECT_TRUE(filter1 == filter2);
  EXPECT_FALSE(filter1 != filter2);
  EXPECT_EQ(filter1, filter2);
}

TEST(BooleanMethodTest, NotEqualOrMethod)
{
  const auto true_filter = TrueMethod{};
  const auto false_filter = FalseMethod{};
  const auto filter1 = OrMethod{true_filter, false_filter};
  const auto filter2 = OrMethod{false_filter, true_filter};

  EXPECT_TRUE(filter1 != filter2);
  EXPECT_FALSE(filter1 == filter2);
  EXPECT_NE(filter1, filter2);
}

TEST(BooleanMethodTest, NotMethod)
{
  const auto true_filter = TrueMethod{};
  const auto false_filter = FalseMethod{};
  const auto asset_data = AssetHistory{};

  const auto not_true = NotMethod{true_filter};
  EXPECT_FALSE(not_true(asset_data, context));

  const auto not_false = NotMethod{false_filter};
  EXPECT_TRUE(not_false(asset_data, context));
}

TEST(BooleanMethodTest, EqualityNotMethod)
{
  const auto true_filter = TrueMethod{};
  const auto not_filter1 = NotMethod{true_filter};
  const auto not_filter2 = NotMethod{true_filter};

  EXPECT_TRUE(not_filter1 == not_filter2);
  EXPECT_FALSE(not_filter1 != not_filter2);
  EXPECT_EQ(not_filter1, not_filter2);
}

TEST(BooleanMethodTest, NotEqualNotMethod)
{
  const auto true_filter = TrueMethod{};
  const auto false_filter = FalseMethod{};
  const auto not_filter1 = NotMethod{true_filter};
  const auto not_filter2 = NotMethod{false_filter};

  EXPECT_TRUE(not_filter1 != not_filter2);
  EXPECT_FALSE(not_filter1 == not_filter2);
  EXPECT_NE(not_filter1, not_filter2);
}

TEST(BooleanMethodTest, XorMethod)
{
  const auto true_filter = TrueMethod{};
  const auto false_filter = FalseMethod{};
  const auto asset_data = AssetHistory{};

  const auto true_xor_true = XorMethod{true_filter, true_filter};
  EXPECT_FALSE(true_xor_true(asset_data, context));

  const auto true_xor_false = XorMethod{true_filter, false_filter};
  EXPECT_TRUE(true_xor_false(asset_data, context));

  const auto false_xor_true = XorMethod{false_filter, true_filter};
  EXPECT_TRUE(false_xor_true(asset_data, context));

  const auto false_xor_false = XorMethod{false_filter, false_filter};
  EXPECT_FALSE(false_xor_false(asset_data, context));
}

TEST(BooleanMethodTest, EqualityXorMethod)
{
  const auto true_filter = TrueMethod{};
  const auto filter1 = XorMethod{true_filter, true_filter};
  const auto filter2 = XorMethod{true_filter, true_filter};

  EXPECT_TRUE(filter1 == filter2);
  EXPECT_FALSE(filter1 != filter2);
  EXPECT_EQ(filter1, filter2);
}

TEST(BooleanMethodTest, NotEqualXorMethod)
{
  const auto true_filter = TrueMethod{};
  const auto false_filter = FalseMethod{};
  const auto filter1 = XorMethod{true_filter, false_filter};
  const auto filter2 = XorMethod{false_filter, true_filter};

  EXPECT_TRUE(filter1 != filter2);
  EXPECT_FALSE(filter1 == filter2);
  EXPECT_NE(filter1, filter2);
}