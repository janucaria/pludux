#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(TrMethodTest, RunAllMethod)
{
  const auto asset_data =
   AssetHistory{{"High", {925, 875, 840, 840, 875, 875, 880, 875, 865, 865}},
                {"Low", {815, 830, 800, 800, 820, 855, 845, 855, 850, 850}},
                {"Close", {875, 830, 800, 835, 870, 875, 860, 860, 860, 855}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};
  const auto tr_method = TrMethod{};

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(tr_method, asset_snapshot[0], context), 15);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(tr_method, asset_snapshot[1], context), 15);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(tr_method, asset_snapshot[2], context), 20);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(tr_method, asset_snapshot[3], context), 35);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(tr_method, asset_snapshot[4], context), 20);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(tr_method, asset_snapshot[5], context), 55);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(tr_method, asset_snapshot[6], context), 40);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(tr_method, asset_snapshot[7], context), 40);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(tr_method, asset_snapshot[8], context), 45);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(tr_method, asset_snapshot[9], context), 110);
}

TEST(TrMethodTest, EqualityOperator)
{
  const auto tr_method1 = TrMethod{};
  const auto tr_method2 = TrMethod{};

  EXPECT_TRUE(tr_method1 == tr_method2);
  EXPECT_EQ(tr_method1, tr_method2);
}
