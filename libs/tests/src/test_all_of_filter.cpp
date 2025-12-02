#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(AllOfMethodTest, AllMethodsPass)
{
  const auto greater_than_condition =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_condition =
   AllOfMethod{{greater_than_condition, less_than_condition}};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(all_of_condition(asset_data, context));
}

TEST(AllOfMethodTest, OneMethodFails)
{
  const auto greater_than_condition =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_condition =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_condition =
   AllOfMethod{{greater_than_condition, less_than_condition}};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_FALSE(all_of_condition(asset_data, context));
}

TEST(AllOfMethodTest, NoMethods)
{
  const auto all_of_condition = AllOfMethod{};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_TRUE(all_of_condition(asset_data, context));
}

TEST(AllOfMethodTest, EqualityOperator)
{
  const auto greater_than_condition =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_condition1 =
   AllOfMethod{{greater_than_condition, less_than_condition}};
  const auto all_of_condition2 =
   AllOfMethod{{greater_than_condition, less_than_condition}};

  EXPECT_TRUE(all_of_condition1 == all_of_condition2);
  EXPECT_FALSE(all_of_condition1 != all_of_condition2);
  EXPECT_EQ(all_of_condition1, all_of_condition2);
}

TEST(AllOfMethodTest, NotEqualOperator)
{
  const auto greater_than_condition1 =
   GreaterThanMethod{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_condition1 =
   LessThanMethod{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_condition1 =
   AllOfMethod{{greater_than_condition1, less_than_condition1}};

  const auto greater_than_condition2 =
   GreaterThanMethod{ValueMethod{25.0}, ValueMethod{15.0}};
  const auto less_than_condition2 =
   LessThanMethod{ValueMethod{35.0}, ValueMethod{55.0}};
  const auto all_of_condition2 =
   AllOfMethod{{greater_than_condition2, less_than_condition2}};

  EXPECT_TRUE(all_of_condition1 != all_of_condition2);
  EXPECT_FALSE(all_of_condition1 == all_of_condition2);
  EXPECT_NE(all_of_condition1, all_of_condition2);
}