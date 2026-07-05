#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(BbMethodTest, ConstructorInitialization)
{
  {
    auto bb_method = BbMethod{};

    EXPECT_EQ(bb_method.ma_method_type(), MaMethodType::Sma);
    EXPECT_EQ(bb_method.source(), CloseMethod{});
    EXPECT_EQ(bb_method.period(), 20);
    EXPECT_EQ(bb_method.stddev(), 1.5);
  }
  {
    auto bb_method = BbMethod{10, 2.0};

    EXPECT_EQ(bb_method.ma_method_type(), MaMethodType::Sma);
    EXPECT_EQ(bb_method.source(), CloseMethod{});
    EXPECT_EQ(bb_method.period(), 10);
    EXPECT_EQ(bb_method.stddev(), 2.0);
  }
  {
    const auto bb_method =
     BbMethod{DataMethod{"close"}, 10, 2.5, MaMethodType::Ema};

    EXPECT_EQ(bb_method.ma_method_type(), MaMethodType::Ema);
    EXPECT_EQ(bb_method.source(), DataMethod{"close"});
    EXPECT_EQ(bb_method.period(), 10);
    EXPECT_EQ(bb_method.stddev(), 2.5);
  }
}

TEST(BbMethodTest, RunAllMethod)
{
  const auto asset_data =
   AssetHistory{{"Close", {875, 830, 800, 835, 870, 875, 860, 860, 860, 855}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  const auto middle_band = BbMethod{5, 2.0};
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[0], context), 862);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[1], context), 865);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[2], context), 860);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[3], context), 848);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[4], context), 842);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(middle_band, asset_snapshot[5], context), 842);
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
   evaluate_series_method(upper_band, asset_snapshot[0], context),
   875.56465996625059);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(upper_band, asset_snapshot[1], context),
   877.64911064067348);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(upper_band, asset_snapshot[2], context),
   887.5680975041804);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(upper_band, asset_snapshot[3], context),
   903.35341001239215);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(upper_band, asset_snapshot[4], context),
   897.35341001239215);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(upper_band, asset_snapshot[5], context),
   897.35341001239215);
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
   evaluate_series_method(lower_band, asset_snapshot[0], context),
   848.43534003374941);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lower_band, asset_snapshot[1], context),
   852.35088935932652);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lower_band, asset_snapshot[2], context),
   832.4319024958196);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lower_band, asset_snapshot[3], context),
   792.64658998760785);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lower_band, asset_snapshot[4], context),
   786.64658998760785);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(lower_band, asset_snapshot[5], context),
   786.64658998760785);
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(lower_band, asset_snapshot[6], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(lower_band, asset_snapshot[7], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(lower_band, asset_snapshot[8], context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(lower_band, asset_snapshot[9], context)));
}

TEST(BbMethodTest, EqualityOperator)
{
  const auto bb_method1 = BbMethod{};
  const auto bb_method2 = BbMethod{};

  EXPECT_TRUE(bb_method1 == bb_method2);
  EXPECT_EQ(bb_method1, bb_method2);
}

TEST(BbMethodTest, NotEqualOperator)
{
  const auto bb_method1 = BbMethod{DataMethod{"close"}, 10, 2.0};
  const auto bb_method2 = BbMethod{DataMethod{"close"}, 10, 1.0};
  const auto bb_method3 = BbMethod{DataMethod{"open"}, 10, 1.0};

  EXPECT_TRUE(bb_method1 != bb_method2);
  EXPECT_NE(bb_method1, bb_method2);
  EXPECT_TRUE(bb_method1 != bb_method3);
  EXPECT_NE(bb_method1, bb_method3);
  EXPECT_TRUE(bb_method2 != bb_method3);
  EXPECT_NE(bb_method2, bb_method3);
}
