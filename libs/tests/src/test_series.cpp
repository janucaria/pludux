#include <gtest/gtest.h>

#include <cmath>

import pludux;

using namespace pludux;

TEST(SeriesTest, ConstructorInitialization)
{
  const auto series = DataSeries{1., 2., 3., 4., 5.};

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

TEST(SeriesTest, LookbackSeries)
{
  const auto series = DataSeries{1., 2., 3., 4., 5.};

  const auto subseries = LookbackSeries{series, 2};
  EXPECT_EQ(subseries.size(), 3);
  EXPECT_EQ(subseries[0], 3);
  EXPECT_EQ(subseries[1], 4);
  EXPECT_EQ(subseries[2], 5);
  EXPECT_TRUE(std::isnan(subseries[3]));
  EXPECT_TRUE(std::isnan(subseries[4]));

  const auto subseries3 = LookbackSeries{series, 0};
  EXPECT_EQ(subseries3.size(), 5);
  EXPECT_EQ(subseries3[0], 1);
  EXPECT_EQ(subseries3[1], 2);
  EXPECT_EQ(subseries3[2], 3);
  EXPECT_EQ(subseries3[3], 4);
  EXPECT_EQ(subseries3[4], 5);
}

TEST(SeriesTest, LookbackSeriesOfLookbackSeries)
{
  const auto series = DataSeries{1., 2., 3., 4., 5.};

  const auto subseries = LookbackSeries{series, 2};
  const auto subsubseries = LookbackSeries{subseries, 1};

  EXPECT_EQ(subsubseries.size(), 2);
  EXPECT_EQ(subsubseries[0], 4);
  EXPECT_EQ(subsubseries[1], 5);
  EXPECT_TRUE(std::isnan(subsubseries[2]));
  EXPECT_TRUE(std::isnan(subsubseries[3]));
  EXPECT_TRUE(std::isnan(subsubseries[4]));
  EXPECT_TRUE(std::isnan(subsubseries[5]));
}

TEST(SeriesTest, LookbackSeriesOfLookbackSeriesWithDifferentTypes)
{
  const auto series = DataSeries{1., 2., 3., 4., 5.};

  const auto subseries = LookbackSeries{series, 2};

  const auto subsubseries = LookbackSeries<PolySeries<double>>{subseries, 1};

  EXPECT_EQ(subsubseries.size(), 2);
  EXPECT_EQ(subsubseries[0], 4);
  EXPECT_EQ(subsubseries[1], 5);
  EXPECT_TRUE(std::isnan(subsubseries[2]));
  EXPECT_TRUE(std::isnan(subsubseries[3]));
  EXPECT_TRUE(std::isnan(subsubseries[4]));
  EXPECT_TRUE(std::isnan(subsubseries[5]));
}