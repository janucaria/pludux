#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(AnyOfMethodTest, AllMethodsPass)
{
  const auto greater_than_condition =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_condition =
   AnyOfMethod{{greater_than_condition, less_than_condition}};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(any_of_condition(asset_data, context));
}

TEST(AnyOfMethodTest, OneMethodFails)
{
  const auto greater_than_condition =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_condition =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_condition =
   AnyOfMethod{{greater_than_condition, less_than_condition}};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(any_of_condition(asset_data, context));
}

TEST(AnyOfMethodTest, AllMethodsFails)
{
  const auto greater_than_condition =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_condition =
   LessThanMethod{ValueMethod{80.0}, ValueMethod{50.0}};
  const auto any_of_condition =
   AnyOfMethod{{greater_than_condition, less_than_condition}};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(any_of_condition(asset_data, context));
}

TEST(AnyOfMethodTest, NoMethods)
{
  const auto any_of_condition = AnyOfMethod{};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(any_of_condition(asset_data, context));
}

TEST(AnyOfMethodTest, EqualityOperator)
{
  const auto greater_than_condition =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_condition1 =
   AnyOfMethod{{greater_than_condition, less_than_condition}};
  const auto any_of_condition2 =
   AnyOfMethod{{greater_than_condition, less_than_condition}};

  EXPECT_TRUE(any_of_condition1 == any_of_condition2);
  EXPECT_FALSE(any_of_condition1 != any_of_condition2);
  EXPECT_EQ(any_of_condition1, any_of_condition2);
}

TEST(AnyOfMethodTest, NotEqualOperator)
{
  const auto greater_than_condition1 =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition1 =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_condition1 =
   AnyOfMethod{{greater_than_condition1, less_than_condition1}};

  const auto greater_than_condition2 =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_condition2 =
   LessThanMethod{ValueMethod{80.0}, ValueMethod{50.0}};
  const auto any_of_condition2 =
   AnyOfMethod{{greater_than_condition2, less_than_condition2}};

  EXPECT_TRUE(any_of_condition1 != any_of_condition2);
  EXPECT_FALSE(any_of_condition1 == any_of_condition2);
  EXPECT_NE(any_of_condition1, any_of_condition2);
}