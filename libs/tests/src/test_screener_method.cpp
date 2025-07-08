import pludux.asset_history;
import pludux.asset_snapshot;
import pludux.screener;
import pludux.series;

#include <gtest/gtest.h>

using namespace pludux::screener;

TEST(ScreenerMethodTest, RunOneMethod)
{
  const auto value_method = ValueMethod{1.0};
  const auto screener_method = ScreenerMethod{value_method};

  const auto casted_method = screener_method_cast<ValueMethod>(screener_method);
  ASSERT_NE(casted_method, nullptr);

  const auto asset_data = pludux::AssetHistory{{"close", {0}}};
  const auto result = screener_method(asset_data)[0];

  EXPECT_DOUBLE_EQ(casted_method->value(), result);
}

TEST(ScreenerMethodTest, EqualityOperator)
{
  const auto value_method1 = ValueMethod{1.0};
  const auto value_method2 = ValueMethod{1.0};
  const auto screener_method1 = ScreenerMethod{value_method1};
  const auto screener_method2 = ScreenerMethod{value_method2};

  EXPECT_TRUE(screener_method1 == screener_method2);
  EXPECT_FALSE(screener_method1 != screener_method2);
  EXPECT_EQ(screener_method1, screener_method2);
}

TEST(ScreenerMethodTest, NotEqualOperator)
{
  const auto value_method1 = ValueMethod{1.0};
  const auto value_method2 = ValueMethod{2.0};
  const auto data_method = DataMethod{"close", 0};
  const auto screener_method1 = ScreenerMethod{value_method1};
  const auto screener_method2 = ScreenerMethod{value_method2};
  const auto screener_method3 = ScreenerMethod{data_method};

  EXPECT_TRUE(screener_method1 != screener_method2);
  EXPECT_FALSE(screener_method1 == screener_method2);
  EXPECT_TRUE(screener_method1 != screener_method3);
  EXPECT_NE(screener_method1, screener_method3);
}
