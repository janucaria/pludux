#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

const auto context = std::monostate{};

TEST(SelectOutputMethodTest, ConstructorInitialization)
{
  auto select_output =
   SelectOutputMethod{CloseMethod{}, SeriesOutput::UpperBand};

  EXPECT_EQ(select_output.output(), SeriesOutput::UpperBand);
  EXPECT_EQ(select_output.source(), CloseMethod{});

  select_output.output(SeriesOutput::LowerBand);
  EXPECT_EQ(select_output.output(), SeriesOutput::LowerBand);

  auto nested_select_output =
   SelectOutputMethod{select_output, SeriesOutput::Histogram};
  EXPECT_EQ(nested_select_output.output(), SeriesOutput::Histogram);
  EXPECT_EQ(nested_select_output.source(), CloseMethod{});
}

TEST(SelectOutputMethodTest, EqualityComparisonOperator)
{
  const auto lookback_method1 =
   SelectOutputMethod{CloseMethod{}, SeriesOutput::UpperBand};
  const auto lookback_method2 =
   SelectOutputMethod{CloseMethod{}, SeriesOutput::UpperBand};
  const auto lookback_method3 =
   SelectOutputMethod{CloseMethod{}, SeriesOutput::LowerBand};

  EXPECT_EQ(lookback_method1, lookback_method2);
  EXPECT_NE(lookback_method1, lookback_method3);
  EXPECT_NE(lookback_method2, lookback_method3);
}