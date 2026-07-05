#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(DonchianChannelMethodTest, ConstructorInitialization)
{
  {
    auto dc_method = DonchianChannelMethod{};
    EXPECT_EQ(dc_method.period(), 20);
  }
  {
    auto dc_method = DonchianChannelMethod{10};
    EXPECT_EQ(dc_method.period(), 10);
  }
}

TEST(DonchianChannelMethodTest, RunAllMethod)
{
  const auto asset_data =
   AssetHistory{{"High", {925, 875, 840, 840, 875, 875, 880, 875, 865, 865}},
                {"Low", {815, 830, 800, 800, 820, 855, 845, 855, 850, 850}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  const auto middle_band = DonchianChannelMethod{5};
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[0], context), 862.5);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[1], context), 850);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[2], context), 840);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[3], context), 840);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[4], context), 837.5);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[5], context), 862.5);
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(middle_band, asset_snapshot[6], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(middle_band, asset_snapshot[7], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(middle_band, asset_snapshot[8], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(middle_band, asset_snapshot[9], context)));

  const auto upper_band =
   SelectOutputMethod{middle_band, MethodOutput::UpperBand};
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(upper_band, asset_snapshot[0], context), 880);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(upper_band, asset_snapshot[1], context), 880);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(upper_band, asset_snapshot[2], context), 880);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(upper_band, asset_snapshot[3], context), 880);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(upper_band, asset_snapshot[4], context), 875);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(upper_band, asset_snapshot[5], context), 925);
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(upper_band, asset_snapshot[6], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(upper_band, asset_snapshot[7], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(upper_band, asset_snapshot[8], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(upper_band, asset_snapshot[9], context)));

  const auto lower_band =
   SelectOutputMethod{middle_band, MethodOutput::LowerBand};
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lower_band, asset_snapshot[0], context), 845);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lower_band, asset_snapshot[1], context), 820);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lower_band, asset_snapshot[2], context), 800);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lower_band, asset_snapshot[3], context), 800);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lower_band, asset_snapshot[4], context), 800);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lower_band, asset_snapshot[5], context), 800);
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(lower_band, asset_snapshot[6], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(lower_band, asset_snapshot[7], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(lower_band, asset_snapshot[8], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(lower_band, asset_snapshot[9], context)));
}

TEST(DonchianChannelMethodTest, EqualityOperator)
{
  const auto dc_method1 = DonchianChannelMethod{};
  const auto dc_method2 = DonchianChannelMethod{};

  EXPECT_TRUE(dc_method1 == dc_method2);
  EXPECT_EQ(dc_method1, dc_method2);
}

TEST(DonchianChannelMethodTest, NotEqualOperator)
{
  const auto dc_method1 = DonchianChannelMethod{5};
  const auto dc_method2 = DonchianChannelMethod{10};
  const auto dc_method3 = DonchianChannelMethod{20};

  EXPECT_TRUE(dc_method1 != dc_method2);
  EXPECT_NE(dc_method1, dc_method2);
  EXPECT_TRUE(dc_method1 != dc_method3);
  EXPECT_NE(dc_method1, dc_method3);
  EXPECT_TRUE(dc_method2 != dc_method3);
  EXPECT_NE(dc_method2, dc_method3);
}
