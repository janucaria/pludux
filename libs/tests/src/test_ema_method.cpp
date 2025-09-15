#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(EmaMethodTest, RunOneMethod)
{
  const auto field_method = DataMethod{"close"};
  const auto period = 5;
  const auto ema_method = EmaMethod{period, field_method};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = ema_method(asset_data)[0];

  EXPECT_DOUBLE_EQ(result, 856.95061728395069);
}

TEST(EmaMethodTest, RunAllMethod)
{
  const auto field_method = DataMethod{"close"};
  const auto period = 5;
  const auto ema_method = EmaMethod{period, field_method};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto series = ema_method(asset_data);

  ASSERT_EQ(series.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(series[0], 856.95061728395069);
  EXPECT_DOUBLE_EQ(series[1], 857.92592592592598);
  EXPECT_DOUBLE_EQ(series[2], 856.88888888888891);
  EXPECT_DOUBLE_EQ(series[3], 855.33333333333337);
  EXPECT_DOUBLE_EQ(series[4], 853);
  EXPECT_DOUBLE_EQ(series[5], 842);
  EXPECT_TRUE(std::isnan(series[6]));
  EXPECT_TRUE(std::isnan(series[7]));
  EXPECT_TRUE(std::isnan(series[8]));
  EXPECT_TRUE(std::isnan(series[9]));
}

TEST(EmaMethodTest, EqualityOperator)
{
  const auto field_method = DataMethod{"close"};
  const auto period = 5;
  const auto ema_method1 = EmaMethod{period, field_method};
  const auto ema_method2 = EmaMethod{period, field_method};

  EXPECT_TRUE(ema_method1 == ema_method2);
  EXPECT_FALSE(ema_method1 != ema_method2);
  EXPECT_EQ(ema_method1, ema_method2);
}

TEST(EmaMethodTest, NotEqualOperator)
{
  const auto field_method1 = DataMethod{"close"};
  const auto period1 = 5;
  const auto ema_method1 = EmaMethod{period1, field_method1};

  const auto field2 = "open";
  const auto field_method2 = DataMethod{field2};
  const auto period2 = 10;
  const auto ema_method2 = EmaMethod{period2, field_method2};

  EXPECT_TRUE(ema_method1 != ema_method2);
  EXPECT_FALSE(ema_method1 == ema_method2);
  EXPECT_NE(ema_method1, ema_method2);
}
