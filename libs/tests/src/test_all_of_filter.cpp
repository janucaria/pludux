#include <gtest/gtest.h>

import pludux;

using namespace pludux;

using SignalGreaterThanMethod =
 pludux::GreaterThanMethod<AnySeriesMethod, AnySeriesMethod>;
using SignalLessThanMethod =
 pludux::LessThanMethod<AnySeriesMethod, AnySeriesMethod>;
using SignalAllOfMethod = pludux::AllOfMethod<AnySeriesMethod>;

const auto context = AnySeriesMethodContext{};

static auto make_asset_snapshot() -> AssetSnapshot
{
  return AssetSnapshot{AssetHistory{{"close", {0}}}};
}

TEST(AllOfMethodTest, AllMethodsPass)
{
  const auto greater_than_condition =
   SignalGreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition =
   SignalLessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_condition =
   SignalAllOfMethod{{greater_than_condition, less_than_condition}};
  const auto asset_snapshot = make_asset_snapshot();

  EXPECT_TRUE(
   evaluate_series_method(all_of_condition, asset_snapshot[0], context));
}

TEST(AllOfMethodTest, OneMethodFails)
{
  const auto greater_than_condition =
   SignalGreaterThanMethod{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_condition =
   SignalLessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_condition =
   SignalAllOfMethod{{greater_than_condition, less_than_condition}};
  const auto asset_snapshot = make_asset_snapshot();

  EXPECT_FALSE(
   evaluate_series_method(all_of_condition, asset_snapshot[0], context));
}

TEST(AllOfMethodTest, NoMethods)
{
  const auto all_of_condition = SignalAllOfMethod{};
  const auto asset_snapshot = make_asset_snapshot();

  EXPECT_TRUE(
   evaluate_series_method(all_of_condition, asset_snapshot[0], context));
}

TEST(AllOfMethodTest, EqualityOperator)
{
  const auto greater_than_condition =
   SignalGreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition =
   SignalLessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_condition1 =
   SignalAllOfMethod{{greater_than_condition, less_than_condition}};
  const auto all_of_condition2 =
   SignalAllOfMethod{{greater_than_condition, less_than_condition}};

  EXPECT_TRUE(all_of_condition1 == all_of_condition2);
  EXPECT_FALSE(all_of_condition1 != all_of_condition2);
  EXPECT_EQ(all_of_condition1, all_of_condition2);
}

TEST(AllOfMethodTest, NotEqualOperator)
{
  const auto greater_than_condition1 =
   SignalGreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition1 =
   SignalLessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_condition1 =
   SignalAllOfMethod{{greater_than_condition1, less_than_condition1}};

  const auto greater_than_condition2 =
   SignalGreaterThanMethod{ValueMethod{25.0}, ValueMethod{15.0}};
  const auto less_than_condition2 =
   SignalLessThanMethod{ValueMethod{35.0}, ValueMethod{55.0}};
  const auto all_of_condition2 =
   SignalAllOfMethod{{greater_than_condition2, less_than_condition2}};

  EXPECT_TRUE(all_of_condition1 != all_of_condition2);
  EXPECT_FALSE(all_of_condition1 == all_of_condition2);
  EXPECT_NE(all_of_condition1, all_of_condition2);
}