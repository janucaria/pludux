#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener/any_of_filter.hpp>
#include <pludux/screener/greater_than_filter.hpp>
#include <pludux/screener/less_than_filter.hpp>
#include <pludux/screener/value_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(AnyOfFilterTest, AllFiltersPass)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20}, ValueMethod{10}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30}, ValueMethod{50}};
  const auto any_of_filter =
   AnyOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset = pludux::Asset{""};

  EXPECT_TRUE(any_of_filter(asset));
}

TEST(AnyOfFilterTest, OneFilterFails)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20}, ValueMethod{30}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{30}, ValueMethod{50}};
  const auto any_of_filter =
   AnyOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset = pludux::Asset{""};

  EXPECT_TRUE(any_of_filter(asset));
}

TEST(AnyOfFilterTest, AllFiltersFails)
{
  const auto greater_than_filter =
   GreaterThanFilter{ValueMethod{20}, ValueMethod{30}};
  const auto less_than_filter =
   LessThanFilter{ValueMethod{80}, ValueMethod{50}};
  const auto any_of_filter =
   AnyOfFilter{{greater_than_filter, less_than_filter}};
  const auto asset = pludux::Asset{""};

  EXPECT_FALSE(any_of_filter(asset));
}

TEST(AnyOfFilterTest, NoFilters)
{
  const auto any_of_filter = AnyOfFilter{};
  const auto asset = pludux::Asset{""};

  EXPECT_FALSE(any_of_filter(asset));
}
