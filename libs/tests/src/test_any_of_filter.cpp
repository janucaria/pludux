#include <gtest/gtest.h>

#include "test_method_context.hpp"

import pludux;

using namespace pludux;

using SignalGreaterThanMethod =
 pludux::GreaterThanMethod<ErasedSeriesMethod<StatelessMethodContext>,
                           ErasedSeriesMethod<StatelessMethodContext>>;
using SignalLessThanMethod =
 pludux::LessThanMethod<ErasedSeriesMethod<StatelessMethodContext>,
                        ErasedSeriesMethod<StatelessMethodContext>>;
using SignalAnyOfMethod =
 pludux::AnyOfMethod<ErasedSeriesMethod<StatelessMethodContext>>;

const auto context = StatelessMethodContext{};

static auto make_asset_snapshot() -> AssetSnapshot
{
  return AssetSnapshot{AssetHistory{{"close", {0}}}};
}

TEST(AnyOfMethodTest, AllMethodsPass)
{
  const auto greater_than_condition =
   SignalGreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition =
   SignalLessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_condition =
   SignalAnyOfMethod{{greater_than_condition, less_than_condition}};
  const auto asset_snapshot = make_asset_snapshot();

  EXPECT_TRUE(
   evaluate_series_method(any_of_condition, asset_snapshot[0], context));
}

TEST(AnyOfMethodTest, OneMethodFails)
{
  const auto greater_than_condition =
   SignalGreaterThanMethod{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_condition =
   SignalLessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_condition =
   SignalAnyOfMethod{{greater_than_condition, less_than_condition}};
  const auto asset_snapshot = make_asset_snapshot();

  EXPECT_TRUE(
   evaluate_series_method(any_of_condition, asset_snapshot[0], context));
}

TEST(AnyOfMethodTest, AllMethodsFails)
{
  const auto greater_than_condition =
   SignalGreaterThanMethod{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_condition =
   SignalLessThanMethod{ValueMethod{80.0}, ValueMethod{50.0}};
  const auto any_of_condition =
   SignalAnyOfMethod{{greater_than_condition, less_than_condition}};
  const auto asset_snapshot = make_asset_snapshot();

  EXPECT_FALSE(
   evaluate_series_method(any_of_condition, asset_snapshot[0], context));
}

TEST(AnyOfMethodTest, NoMethods)
{
  const auto any_of_condition = SignalAnyOfMethod{};
  const auto asset_snapshot = make_asset_snapshot();

  EXPECT_FALSE(
   evaluate_series_method(any_of_condition, asset_snapshot[0], context));
}

TEST(AnyOfMethodTest, EqualityOperator)
{
  const auto greater_than_condition =
   SignalGreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition =
   SignalLessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_condition1 =
   SignalAnyOfMethod{{greater_than_condition, less_than_condition}};
  const auto any_of_condition2 =
   SignalAnyOfMethod{{greater_than_condition, less_than_condition}};

  EXPECT_TRUE(any_of_condition1 == any_of_condition2);
  EXPECT_FALSE(any_of_condition1 != any_of_condition2);
  EXPECT_EQ(any_of_condition1, any_of_condition2);
}

TEST(AnyOfMethodTest, NotEqualOperator)
{
  const auto greater_than_condition1 =
   SignalGreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition1 =
   SignalLessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_condition1 =
   SignalAnyOfMethod{{greater_than_condition1, less_than_condition1}};

  const auto greater_than_condition2 =
   SignalGreaterThanMethod{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_condition2 =
   SignalLessThanMethod{ValueMethod{80.0}, ValueMethod{50.0}};
  const auto any_of_condition2 =
   SignalAnyOfMethod{{greater_than_condition2, less_than_condition2}};

  EXPECT_TRUE(any_of_condition1 != any_of_condition2);
  EXPECT_FALSE(any_of_condition1 == any_of_condition2);
  EXPECT_NE(any_of_condition1, any_of_condition2);
}