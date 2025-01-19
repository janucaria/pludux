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
  EXPECT_EQ(subseries.size(), 5);
  EXPECT_EQ(subseries[0], 3);
  EXPECT_EQ(subseries[1], 4);
  EXPECT_EQ(subseries[2], 5);
  EXPECT_TRUE(std::isnan(subseries[3]));
  EXPECT_TRUE(std::isnan(subseries[4]));

  const auto subseries2 = SubSeries{series, -2};
  EXPECT_EQ(subseries2.size(), 5);
  EXPECT_TRUE(std::isnan(subseries2[0]));
  EXPECT_TRUE(std::isnan(subseries2[1]));
  EXPECT_EQ(subseries2[2], 1);
  EXPECT_EQ(subseries2[3], 2);
  EXPECT_EQ(subseries2[4], 3);
  EXPECT_TRUE(std::isnan(subseries2[5]));

  const auto subseries3 = SubSeries{series, 0};
  EXPECT_EQ(subseries3.size(), 5);
  EXPECT_EQ(subseries3[0], 1);
  EXPECT_EQ(subseries3[1], 2);
  EXPECT_EQ(subseries3[2], 3);
  EXPECT_EQ(subseries3[3], 4);
  EXPECT_EQ(subseries3[4], 5);
}

TEST(SeriesTest, SubSeriesOfSubSeries)
{
  const auto series = DataSeries{1., 2., 3., 4., 5.};

  const auto subseries = SubSeries{series, 2};
  const auto subsubseries = SubSeries{subseries, 1};

  EXPECT_EQ(subsubseries.size(), 5);
  EXPECT_EQ(subsubseries[0], 4);
  EXPECT_EQ(subsubseries[1], 5);
  EXPECT_TRUE(std::isnan(subsubseries[2]));
  EXPECT_TRUE(std::isnan(subsubseries[3]));
  EXPECT_TRUE(std::isnan(subsubseries[4]));
  EXPECT_TRUE(std::isnan(subsubseries[5]));

  const auto subsubseries2 = SubSeries{subsubseries, -3};
  EXPECT_EQ(subsubseries2.size(), 5);
  EXPECT_EQ(subsubseries2[0], 1);
  EXPECT_EQ(subsubseries2[1], 2);
  EXPECT_EQ(subsubseries2[2], 3);
  EXPECT_EQ(subsubseries2[3], 4);
  EXPECT_EQ(subsubseries2[4], 5);
}

TEST(SeriesTest, SubSeriesOfSubSeriesWithDifferentTypes)
{
  const auto series = DataSeries{1., 2., 3., 4., 5.};

  const auto subseries = SubSeries{series, 3};

  const auto subsubseries = SubSeries<PolySeries<double>>{subseries, -3};

  EXPECT_EQ(subsubseries.size(), 5);
  EXPECT_EQ(subsubseries[0], 1);
  EXPECT_EQ(subsubseries[1], 2);
  EXPECT_EQ(subsubseries[2], 3);
  EXPECT_EQ(subsubseries[3], 4);
  EXPECT_EQ(subsubseries[4], 5);
}