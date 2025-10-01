#include <gtest/gtest.h>

import pludux;

using namespace pludux::screener;

const auto context = AnyMethodContext{};

TEST(AnyOfFilterTest, AllFiltersPass)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_filter =
   AnyOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_TRUE(any_of_filter(asset_data, context));
}

TEST(AnyOfFilterTest, OneFilterFails)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_filter =
   AnyOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_TRUE(any_of_filter(asset_data, context));
}

TEST(AnyOfFilterTest, AllFiltersFails)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{80.0}, ValueMethod{50.0}};
  const auto any_of_filter =
   AnyOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_FALSE(any_of_filter(asset_data, context));
}

TEST(AnyOfFilterTest, NoFilters)
{
  const auto any_of_filter = AnyOfFilter{};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_FALSE(any_of_filter(asset_data, context));
}

TEST(AnyOfFilterTest, EqualityOperator)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_filter1 =
   AnyOfFilter{{greater_than_filter, less_than_filter}};
  const auto any_of_filter2 =
   AnyOfFilter{{greater_than_filter, less_than_filter}};

  EXPECT_TRUE(any_of_filter1 == any_of_filter2);
  EXPECT_FALSE(any_of_filter1 != any_of_filter2);
  EXPECT_EQ(any_of_filter1, any_of_filter2);
}

TEST(AnyOfFilterTest, NotEqualOperator)
{
  const auto greater_than_filter1 =
   GreaterThanFilter{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_filter1 =
   LessThanFilter{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto any_of_filter1 =
   AnyOfFilter{{greater_than_filter1, less_than_filter1}};

  const auto greater_than_filter2 =
   GreaterThanFilter{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_filter2 =
   LessThanFilter{ValueMethod{80.0}, ValueMethod{50.0}};
  const auto any_of_filter2 =
   AnyOfFilter{{greater_than_filter2, less_than_filter2}};

  EXPECT_TRUE(any_of_filter1 != any_of_filter2);
  EXPECT_FALSE(any_of_filter1 == any_of_filter2);
  EXPECT_NE(any_of_filter1, any_of_filter2);
}