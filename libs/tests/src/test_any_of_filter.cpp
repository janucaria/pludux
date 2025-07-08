import pludux.asset_history;
import pludux.asset_snapshot;
import pludux.screener;
import pludux.series;

#include <gtest/gtest.h>
#include <pludux/screener.hpp>

using namespace pludux::screener;

TEST(AnyOfFilterTest, AllFiltersPass)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20}, ValueMethod{10}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30}, ValueMethod{50}};
  const auto any_of_filter =
   AnyOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_TRUE(any_of_filter(asset_data));
}

TEST(AnyOfFilterTest, OneFilterFails)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20}, ValueMethod{30}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30}, ValueMethod{50}};
  const auto any_of_filter =
   AnyOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_TRUE(any_of_filter(asset_data));
}

TEST(AnyOfFilterTest, AllFiltersFails)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20}, ValueMethod{30}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{80}, ValueMethod{50}};
  const auto any_of_filter =
   AnyOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_FALSE(any_of_filter(asset_data));
}

TEST(AnyOfFilterTest, NoFilters)
{
  const auto any_of_filter = AnyOfFilter{};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_FALSE(any_of_filter(asset_data));
}

TEST(AnyOfFilterTest, EqualityOperator)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20}, ValueMethod{10}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30}, ValueMethod{50}};
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
   GreaterThanFilter{ValueMethod{20}, ValueMethod{10}};
  const auto less_than_filter1 =
   LessThanFilter{ValueMethod{30}, ValueMethod{50}};
  const auto any_of_filter1 =
   AnyOfFilter{{greater_than_filter1, less_than_filter1}};

  const auto greater_than_filter2 =
   GreaterThanFilter{ValueMethod{20}, ValueMethod{30}};
  const auto less_than_filter2 =
   LessThanFilter{ValueMethod{80}, ValueMethod{50}};
  const auto any_of_filter2 =
   AnyOfFilter{{greater_than_filter2, less_than_filter2}};

  EXPECT_TRUE(any_of_filter1 != any_of_filter2);
  EXPECT_FALSE(any_of_filter1 == any_of_filter2);
  EXPECT_NE(any_of_filter1, any_of_filter2);
}