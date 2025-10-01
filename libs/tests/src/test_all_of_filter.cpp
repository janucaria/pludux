#include <gtest/gtest.h>

import pludux;

using namespace pludux::screener;

const auto context = AnyMethodContext{};

TEST(AllOfFilterTest, AllFiltersPass)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_filter =
   AllOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_TRUE(all_of_filter(asset_data, context));
}

TEST(AllOfFilterTest, OneFilterFails)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20.0}, ValueMethod{30.0}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_filter =
   AllOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_FALSE(all_of_filter(asset_data, context));
}

TEST(AllOfFilterTest, NoFilters)
{
  const auto all_of_filter = AllOfFilter{};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_TRUE(all_of_filter(asset_data, context));
}

TEST(AllOfFilterTest, EqualityOperator)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_filter1 =
   AllOfFilter{{greater_than_filter, less_than_filter}};
  const auto all_of_filter2 =
   AllOfFilter{{greater_than_filter, less_than_filter}};

  EXPECT_TRUE(all_of_filter1 == all_of_filter2);
  EXPECT_FALSE(all_of_filter1 != all_of_filter2);
  EXPECT_EQ(all_of_filter1, all_of_filter2);
}

TEST(AllOfFilterTest, NotEqualOperator)
{
  const auto greater_than_filter1 =
   GreaterThanFilter{ValueMethod{20.0}, ValueMethod{10.0}};
  const auto less_than_filter1 =
   LessThanFilter{ValueMethod{30.0}, ValueMethod{50.0}};
  const auto all_of_filter1 =
   AllOfFilter{{greater_than_filter1, less_than_filter1}};

  const auto greater_than_filter2 =
   GreaterThanFilter{ValueMethod{25.0}, ValueMethod{15.0}};
  const auto less_than_filter2 =
   LessThanFilter{ValueMethod{35.0}, ValueMethod{55.0}};
  const auto all_of_filter2 =
   AllOfFilter{{greater_than_filter2, less_than_filter2}};

  EXPECT_TRUE(all_of_filter1 != all_of_filter2);
  EXPECT_FALSE(all_of_filter1 == all_of_filter2);
  EXPECT_NE(all_of_filter1, all_of_filter2);
}