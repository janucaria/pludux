#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(AnyOfMethodTest, AllMethodsPass)
{
  const auto greater_than_filter =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_filter =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_filter =
   AnyOfMethod{{greater_than_filter, less_than_filter}};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(any_of_filter(asset_data, context));
}

TEST(AnyOfMethodTest, OneMethodFails)
{
  const auto greater_than_filter =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_filter =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_filter =
   AnyOfMethod{{greater_than_filter, less_than_filter}};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(any_of_filter(asset_data, context));
}

TEST(AnyOfMethodTest, AllMethodsFails)
{
  const auto greater_than_filter =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_filter =
   LessThanMethod{ValueMethod{80.0}, ValueMethod{50.0}};
  const auto any_of_filter =
   AnyOfMethod{{greater_than_filter, less_than_filter}};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(any_of_filter(asset_data, context));
}

TEST(AnyOfMethodTest, NoMethods)
{
  const auto any_of_filter = AnyOfMethod{};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(any_of_filter(asset_data, context));
}

TEST(AnyOfMethodTest, EqualityOperator)
{
  const auto greater_than_filter =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_filter =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_filter1 =
   AnyOfMethod{{greater_than_filter, less_than_filter}};
  const auto any_of_filter2 =
   AnyOfMethod{{greater_than_filter, less_than_filter}};

  EXPECT_TRUE(any_of_filter1 == any_of_filter2);
  EXPECT_FALSE(any_of_filter1 != any_of_filter2);
  EXPECT_EQ(any_of_filter1, any_of_filter2);
}

TEST(AnyOfMethodTest, NotEqualOperator)
{
  const auto greater_than_filter1 =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_filter1 =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_filter1 =
   AnyOfMethod{{greater_than_filter1, less_than_filter1}};

  const auto greater_than_filter2 =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_filter2 =
   LessThanMethod{ValueMethod{80.0}, ValueMethod{50.0}};
  const auto any_of_filter2 =
   AnyOfMethod{{greater_than_filter2, less_than_filter2}};

  EXPECT_TRUE(any_of_filter1 != any_of_filter2);
  EXPECT_FALSE(any_of_filter1 == any_of_filter2);
  EXPECT_NE(any_of_filter1, any_of_filter2);
}