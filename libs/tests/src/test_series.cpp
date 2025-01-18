#include <gtest/gtest.h>
#include <limits>
#include <pludux/series.hpp>
#include <vector>

using namespace pludux;

TEST(SeriesTest, ConstructorInitialization)
{
  const auto series = DataSeries{1, 2, 3, 4, 5};

  ASSERT_EQ(series.size(), 5);
}

TEST(SeriesTest, OperatorIndex)
{
  const auto series = DataSeries{1., 2., 3., 4., 5.};

  EXPECT_EQ(series[0], 1.);
  EXPECT_EQ(series[1], 2.);
  EXPECT_EQ(series[2], 3.);
  EXPECT_EQ(series[3], 4.);
  EXPECT_EQ(series[4], 5.);
  EXPECT_TRUE(std::isnan(series[5]));
}

TEST(SeriesTest, SizeFunction)
{
  const auto data = std::vector<double>{1, 2, 3, 4, 5};
  const auto series = DataSeries(data.begin(), data.end());

  EXPECT_EQ(series.size(), data.size());
}

TEST(SeriesTest, EmptySeries)
{
  const auto series = DataSeries<double>{};

  EXPECT_EQ(series.size(), 0);
  EXPECT_TRUE(std::isnan(series[0]));
}

TEST(SeriesTest, Subseries)
{
  const auto series = DataSeries{1., 2., 3., 4., 5.};

  const auto subseries = SubSeries{series, 2};
  EXPECT_EQ(subseries.size(), 3);
  EXPECT_EQ(subseries[0], 3);
  EXPECT_EQ(subseries[1], 4);
  EXPECT_EQ(subseries[2], 5);
  EXPECT_TRUE(std::isnan(subseries[3]));
}