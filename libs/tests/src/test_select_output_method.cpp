#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux::screener;

const auto context = std::monostate{};

TEST(SelectOutputMethodTest, ConstructorInitialization)
{
  auto select_output =
   SelectOutputMethod{CloseMethod{}, MethodOutput::UpperBand};

  EXPECT_EQ(select_output.output(), MethodOutput::UpperBand);
  EXPECT_EQ(select_output.source(), CloseMethod{});

  select_output.output(MethodOutput::LowerBand);
  EXPECT_EQ(select_output.output(), MethodOutput::LowerBand);

  auto nested_select_output =
   SelectOutputMethod{select_output, MethodOutput::Histogram};
  EXPECT_EQ(nested_select_output.output(), MethodOutput::Histogram);
  EXPECT_EQ(nested_select_output.source(), CloseMethod{});
}

TEST(SelectOutputMethodTest, EqualityComparisonOperator)
{
  const auto lookback_method1 =
   SelectOutputMethod{CloseMethod{}, MethodOutput::UpperBand};
  const auto lookback_method2 =
   SelectOutputMethod{CloseMethod{}, MethodOutput::UpperBand};
  const auto lookback_method3 =
   SelectOutputMethod{CloseMethod{}, MethodOutput::LowerBand};

  EXPECT_EQ(lookback_method1, lookback_method2);
  EXPECT_NE(lookback_method1, lookback_method3);
  EXPECT_NE(lookback_method2, lookback_method3);
}