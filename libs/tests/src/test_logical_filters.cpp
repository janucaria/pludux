#include <gtest/gtest.h>

import pludux;

using namespace pludux;

using SignalAlwaysMethod = pludux::TrueMethod;
using SignalNeverMethod = pludux::FalseMethod;
using SignalAndMethod =
 pludux::LogicalAndMethod<ErasedSeriesMethod<ErasedSeriesMethodContext>,
                          ErasedSeriesMethod<ErasedSeriesMethodContext>>;
using SignalOrMethod =
 pludux::LogicalOrMethod<ErasedSeriesMethod<ErasedSeriesMethodContext>,
                         ErasedSeriesMethod<ErasedSeriesMethodContext>>;
using SignalNotMethod =
 pludux::LogicalNotMethod<ErasedSeriesMethod<ErasedSeriesMethodContext>>;
using SignalXorMethod =
 pludux::LogicalXorMethod<ErasedSeriesMethod<ErasedSeriesMethodContext>,
                          ErasedSeriesMethod<ErasedSeriesMethodContext>>;

const auto context = ErasedSeriesMethodContext{};

static auto make_asset_snapshot() -> AssetSnapshot
{
  return AssetSnapshot{AssetHistory{{"close", {0}}}};
}

TEST(BooleanMethodTest, AndMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto false_condition = SignalNeverMethod{};
  const auto asset_snapshot = make_asset_snapshot();

  const auto true_and_true = SignalAndMethod{true_condition, true_condition};
  EXPECT_TRUE(
   evaluate_series_method(true_and_true, asset_snapshot[0], context));

  const auto true_and_false = SignalAndMethod{true_condition, false_condition};
  EXPECT_FALSE(
   evaluate_series_method(true_and_false, asset_snapshot[0], context));

  const auto false_and_true = SignalAndMethod{false_condition, true_condition};
  EXPECT_FALSE(
   evaluate_series_method(false_and_true, asset_snapshot[0], context));

  const auto false_and_false =
   SignalAndMethod{false_condition, false_condition};
  EXPECT_FALSE(
   evaluate_series_method(false_and_false, asset_snapshot[0], context));
}

TEST(BooleanMethodTest, EqualityAndMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto condition1 = SignalAndMethod{true_condition, true_condition};
  const auto condition2 = SignalAndMethod{true_condition, true_condition};

  EXPECT_TRUE(condition1 == condition2);
  EXPECT_FALSE(condition1 != condition2);
  EXPECT_EQ(condition1, condition2);
}

TEST(BooleanMethodTest, NotEqualAndMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto false_condition = SignalNeverMethod{};
  const auto condition1 = SignalAndMethod{true_condition, false_condition};
  const auto condition2 = SignalAndMethod{false_condition, true_condition};

  EXPECT_TRUE(condition1 != condition2);
  EXPECT_FALSE(condition1 == condition2);
  EXPECT_NE(condition1, condition2);
}

TEST(BooleanMethodTest, OrMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto false_condition = SignalNeverMethod{};
  const auto asset_snapshot = make_asset_snapshot();

  const auto true_or_true = SignalOrMethod{true_condition, true_condition};
  EXPECT_TRUE(evaluate_series_method(true_or_true, asset_snapshot[0], context));

  const auto true_or_false = SignalOrMethod{true_condition, false_condition};
  EXPECT_TRUE(
   evaluate_series_method(true_or_false, asset_snapshot[0], context));

  const auto false_or_true = SignalOrMethod{false_condition, true_condition};
  EXPECT_TRUE(
   evaluate_series_method(false_or_true, asset_snapshot[0], context));

  const auto false_or_false = SignalOrMethod{false_condition, false_condition};
  EXPECT_FALSE(
   evaluate_series_method(false_or_false, asset_snapshot[0], context));
}

TEST(BooleanMethodTest, EqualityOrMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto condition1 = SignalOrMethod{true_condition, true_condition};
  const auto condition2 = SignalOrMethod{true_condition, true_condition};

  EXPECT_TRUE(condition1 == condition2);
  EXPECT_FALSE(condition1 != condition2);
  EXPECT_EQ(condition1, condition2);
}

TEST(BooleanMethodTest, NotEqualOrMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto false_condition = SignalNeverMethod{};
  const auto condition1 = SignalOrMethod{true_condition, false_condition};
  const auto condition2 = SignalOrMethod{false_condition, true_condition};

  EXPECT_TRUE(condition1 != condition2);
  EXPECT_FALSE(condition1 == condition2);
  EXPECT_NE(condition1, condition2);
}

TEST(BooleanMethodTest, NotMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto false_condition = SignalNeverMethod{};
  const auto asset_snapshot = make_asset_snapshot();

  const auto not_true = SignalNotMethod{true_condition};
  EXPECT_FALSE(evaluate_series_method(not_true, asset_snapshot[0], context));

  const auto not_false = SignalNotMethod{false_condition};
  EXPECT_TRUE(evaluate_series_method(not_false, asset_snapshot[0], context));
}

TEST(BooleanMethodTest, EqualityNotMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto not_condition1 = SignalNotMethod{true_condition};
  const auto not_condition2 = SignalNotMethod{true_condition};

  EXPECT_TRUE(not_condition1 == not_condition2);
  EXPECT_FALSE(not_condition1 != not_condition2);
  EXPECT_EQ(not_condition1, not_condition2);
}

TEST(BooleanMethodTest, NotEqualNotMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto false_condition = SignalNeverMethod{};
  const auto not_condition1 = SignalNotMethod{true_condition};
  const auto not_condition2 = SignalNotMethod{false_condition};

  EXPECT_TRUE(not_condition1 != not_condition2);
  EXPECT_FALSE(not_condition1 == not_condition2);
  EXPECT_NE(not_condition1, not_condition2);
}

TEST(BooleanMethodTest, XorMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto false_condition = SignalNeverMethod{};
  const auto asset_snapshot = make_asset_snapshot();

  const auto true_xor_true = SignalXorMethod{true_condition, true_condition};
  EXPECT_FALSE(
   evaluate_series_method(true_xor_true, asset_snapshot[0], context));

  const auto true_xor_false = SignalXorMethod{true_condition, false_condition};
  EXPECT_TRUE(
   evaluate_series_method(true_xor_false, asset_snapshot[0], context));

  const auto false_xor_true = SignalXorMethod{false_condition, true_condition};
  EXPECT_TRUE(
   evaluate_series_method(false_xor_true, asset_snapshot[0], context));

  const auto false_xor_false =
   SignalXorMethod{false_condition, false_condition};
  EXPECT_FALSE(
   evaluate_series_method(false_xor_false, asset_snapshot[0], context));
}

TEST(BooleanMethodTest, EqualityXorMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto condition1 = SignalXorMethod{true_condition, true_condition};
  const auto condition2 = SignalXorMethod{true_condition, true_condition};

  EXPECT_TRUE(condition1 == condition2);
  EXPECT_FALSE(condition1 != condition2);
  EXPECT_EQ(condition1, condition2);
}

TEST(BooleanMethodTest, NotEqualXorMethod)
{
  const auto true_condition = SignalAlwaysMethod{};
  const auto false_condition = SignalNeverMethod{};
  const auto condition1 = SignalXorMethod{true_condition, false_condition};
  const auto condition2 = SignalXorMethod{false_condition, true_condition};

  EXPECT_TRUE(condition1 != condition2);
  EXPECT_FALSE(condition1 == condition2);
  EXPECT_NE(condition1, condition2);
}