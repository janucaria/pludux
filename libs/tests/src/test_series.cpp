#include <gtest/gtest.h>
#include <limits>
#include <vector>
#include <pludux/series.hpp>

using namespace pludux;

TEST(SeriesTest, ConstructorInitialization)
{
  const auto data = std::vector<double>{1, 2, 3, 4, 5};
  const auto series = DataSeries{data};

  ASSERT_EQ(series.size(), data.size());
  for (std::size_t i = 0; i < data.size(); ++i) {
    EXPECT_EQ(series[i], data[data.size() - 1 - i]);
  }
}

TEST(SeriesTest, OperatorIndex)
{
  const auto data = std::vector<double>{1, 2, 3, 4, 5};
  const auto series = DataSeries{data};

  EXPECT_EQ(series[0], 5);
  EXPECT_EQ(series[1], 4);
  EXPECT_EQ(series[2], 3);
  EXPECT_EQ(series[3], 2);
  EXPECT_EQ(series[4], 1);
  EXPECT_TRUE(std::isnan(series[5]));
}

TEST(SeriesTest, SizeFunction)
{
  const auto data = std::vector<double>{1, 2, 3, 4, 5};
  const auto series = DataSeries{data};

  EXPECT_EQ(series.size(), data.size());
}

TEST(SeriesTest, EmptySeries)
{
  const auto data = std::vector<double>{};
  const auto series = DataSeries{data};

  EXPECT_EQ(series.size(), 0);
  EXPECT_TRUE(std::isnan(series[0]));
}

TEST(SeriesTest, Subseries)
{
  const auto data = std::vector<double>{1, 2, 3, 4, 5};
  const auto series = DataSeries{data};

  const auto subseries = SubSeries{series, 2};
  EXPECT_EQ(subseries.size(), 3);
  EXPECT_EQ(subseries[0], 3);
  EXPECT_EQ(subseries[1], 2);
  EXPECT_EQ(subseries[2], 1);
}