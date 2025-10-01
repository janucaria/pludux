#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;
using namespace pludux::screener;

TEST(KcMethodTest, middle_EMA_range_ATR)
{
  const auto ma_method = EmaMethod{5};
  const auto range_method = AtrMethod{4};
  const auto multiplier = 2.0;
  const auto asset_data = pludux::AssetHistory{
   {"High", {865, 865, 875, 880, 875, 875, 840, 840, 875, 925}},
   {"Low", {850, 850, 855, 845, 855, 820, 800, 800, 830, 815}},
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  const auto kc_middle = KcMethod{ma_method, range_method, multiplier};
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[0], context), 856.95061728395069);
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[1], context), 857.92592592592598);
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[2], context), 856.88888888888891);
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[3], context), 855.33333333333337);
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[4], context), 853);
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[5], context), 842);
  EXPECT_TRUE(std::isnan(kc_middle(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(kc_middle(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(kc_middle(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(kc_middle(asset_snapshot[9], context)));

  const auto kc_upper = SelectOutputMethod{kc_middle, MethodOutput::UpperBand};
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[0], context), 913.68584677613819);
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[1], context), 923.57289858217598);
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[2], context), 934.41818576388891);
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[3], context), 945.37239583333337);
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[4], context), 949.71875);
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[5], context), 957.625);
  EXPECT_TRUE(std::isnan(kc_upper(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(kc_upper(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(kc_upper(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(kc_upper(asset_snapshot[9], context)));

  const auto kc_lower = SelectOutputMethod{kc_upper, MethodOutput::LowerBand};
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[0], context), 800.21538779176319);
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[1], context), 792.27895326967598);
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[2], context), 779.35959201388891);
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[3], context), 765.29427083333337);
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[4], context), 756.28125);
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[5], context), 726.375);
  EXPECT_TRUE(std::isnan(kc_lower(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(kc_lower(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(kc_lower(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(kc_lower(asset_snapshot[9], context)));
}

TEST(KcMethodTest, middle_SMA_range_Range)
{
  const auto ma_period = 5;
  const auto ma_method = SmaMethod{CloseMethod{}, ma_period};
  const auto range_method =
   RmaMethod{SubtractMethod{HighMethod{}, LowMethod{}}, ma_period};
  const auto multiplier = 1.0;
  const auto asset_data = pludux::AssetHistory{
   {"High", {865, 865, 875, 880, 875, 875, 840, 840, 875, 925}},
   {"Low", {850, 850, 855, 845, 855, 820, 800, 800, 830, 815}},
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  const auto kc_middle = KcMethod{ma_method, range_method, multiplier};
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[0], context), 862);
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[1], context), 865);
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[2], context), 860);
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[3], context), 848);
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[4], context), 842);
  EXPECT_DOUBLE_EQ(kc_middle(asset_snapshot[5], context), 842);
  EXPECT_TRUE(std::isnan(kc_middle(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(kc_middle(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(kc_middle(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(kc_middle(asset_snapshot[9], context)));

  const auto kc_upper = SelectOutputMethod{kc_middle, MethodOutput::UpperBand};
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[0], context), 894.18784000000005);
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[1], context), 901.48479999999995);
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[2], context), 901.85599999999999);
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[3], context), 895.32000000000005);
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[4], context), 892.39999999999998);
  EXPECT_DOUBLE_EQ(kc_upper(asset_snapshot[5], context), 900.0);
  EXPECT_TRUE(std::isnan(kc_upper(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(kc_upper(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(kc_upper(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(kc_upper(asset_snapshot[9], context)));

  const auto kc_lower = SelectOutputMethod{kc_middle, MethodOutput::LowerBand};
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[0], context), 829.81215999999995);
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[1], context), 828.51520000000005);
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[2], context), 818.14400000000001);
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[3], context), 800.67999999999995);
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[4], context), 791.60000000000002);
  EXPECT_DOUBLE_EQ(kc_lower(asset_snapshot[5], context), 784.0);
  EXPECT_TRUE(std::isnan(kc_lower(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(kc_lower(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(kc_lower(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(kc_lower(asset_snapshot[9], context)));
}

TEST(KcMethodTest, EqualityOperator)
{
  const auto kc_method1 = KcMethod{};
  const auto kc_method2 = KcMethod{};

  EXPECT_TRUE(kc_method1 == kc_method2);
  EXPECT_EQ(kc_method1, kc_method2);
}

TEST(KcMethodTest, NotEqualOperator)
{
  const auto kc_method1 = KcMethod{EmaMethod{5}, AtrMethod{14}, 2.0};
  const auto kc_method2 = KcMethod{EmaMethod{5}, AtrMethod{20}, 2.0};
  const auto kc_method3 = KcMethod{EmaMethod{9}, AtrMethod{14}, 2.0};

  EXPECT_TRUE(kc_method1 != kc_method2);
  EXPECT_NE(kc_method1, kc_method2);
  EXPECT_TRUE(kc_method1 != kc_method3);
  EXPECT_NE(kc_method1, kc_method3);
  EXPECT_TRUE(kc_method2 != kc_method3);
  EXPECT_NE(kc_method2, kc_method3);
}
