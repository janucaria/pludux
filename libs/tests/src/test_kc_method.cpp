#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/arithmetic_method.hpp>
#include <pludux/screener/atr_method.hpp>
#include <pludux/screener/data_method.hpp>
#include <pludux/screener/kc_method.hpp>
#include <pludux/screener/ta_with_period_method.hpp>
#include <pludux/series.hpp>

using namespace pludux;
using namespace pludux::screener;

TEST(KcMethodTest, middle_EMA_range_ATR)
{
  const auto data_method = DataMethod{"close"};
  const auto ma_method = EmaMethod{5, data_method, 0};
  const auto range_method = AtrMethod{4};
  const auto multiplier = 2.0;
  const auto asset_data = pludux::AssetHistory{
   {"high", {865, 865, 875, 880, 875, 875, 840, 840, 875, 925}},
   {"low", {850, 850, 855, 845, 855, 820, 800, 800, 830, 815}},
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  auto kc_method =
   KcMethod{KcOutput::middle, ma_method, range_method, multiplier};

  const auto kc_middle = kc_method(asset_data);
  const auto expected_middle = ma_method(asset_data);
  ASSERT_EQ(kc_middle.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(kc_middle[0], expected_middle[0]);
  EXPECT_DOUBLE_EQ(kc_middle[1], expected_middle[1]);
  EXPECT_DOUBLE_EQ(kc_middle[2], expected_middle[2]);
  EXPECT_DOUBLE_EQ(kc_middle[3], expected_middle[3]);
  EXPECT_DOUBLE_EQ(kc_middle[4], expected_middle[4]);
  EXPECT_DOUBLE_EQ(kc_middle[5], expected_middle[5]);
  EXPECT_TRUE(std::isnan(kc_middle[6]) && std::isnan(expected_middle[6]));
  EXPECT_TRUE(std::isnan(kc_middle[7]) && std::isnan(expected_middle[7]));
  EXPECT_TRUE(std::isnan(kc_middle[8]) && std::isnan(expected_middle[8]));
  EXPECT_TRUE(std::isnan(kc_middle[9]) && std::isnan(expected_middle[9]));

  kc_method.output(KcOutput::upper);
  const auto kc_upper = kc_method(asset_data);
  ASSERT_EQ(kc_upper.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(kc_upper[0], 913.68584677613819);
  EXPECT_DOUBLE_EQ(kc_upper[1], 923.57289858217598);
  EXPECT_DOUBLE_EQ(kc_upper[2], 934.41818576388891);
  EXPECT_DOUBLE_EQ(kc_upper[3], 945.37239583333337);
  EXPECT_DOUBLE_EQ(kc_upper[4], 949.71875);
  EXPECT_DOUBLE_EQ(kc_upper[5], 957.625);
  EXPECT_TRUE(std::isnan(kc_upper[6]));
  EXPECT_TRUE(std::isnan(kc_upper[7]));
  EXPECT_TRUE(std::isnan(kc_upper[8]));
  EXPECT_TRUE(std::isnan(kc_upper[9]));

  kc_method.output(KcOutput::lower);
  const auto kc_lower = kc_method(asset_data);
  ASSERT_EQ(kc_lower.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(kc_lower[0], 800.21538779176319);
  EXPECT_DOUBLE_EQ(kc_lower[1], 792.27895326967598);
  EXPECT_DOUBLE_EQ(kc_lower[2], 779.35959201388891);
  EXPECT_DOUBLE_EQ(kc_lower[3], 765.29427083333337);
  EXPECT_DOUBLE_EQ(kc_lower[4], 756.28125);
  EXPECT_DOUBLE_EQ(kc_lower[5], 726.375);
  EXPECT_TRUE(std::isnan(kc_lower[6]));
  EXPECT_TRUE(std::isnan(kc_lower[7]));
  EXPECT_TRUE(std::isnan(kc_lower[8]));
  EXPECT_TRUE(std::isnan(kc_lower[9]));
}

TEST(KcMethodTest, middle_SMA_range_Range)
{
  const auto data_method = DataMethod{"close"};
  const auto ma_period = 5;
  const auto ma_method = SmaMethod{ma_period, data_method, 0};
  const auto range_method = RmaMethod{
   ma_period, SubtractMethod{DataMethod{"high"}, DataMethod{"low"}}, 0};
  const auto multiplier = 1.0;
  const auto asset_data = pludux::AssetHistory{
   {"high", {865, 865, 875, 880, 875, 875, 840, 840, 875, 925}},
   {"low", {850, 850, 855, 845, 855, 820, 800, 800, 830, 815}},
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  auto kc_method =
   KcMethod{KcOutput::middle, ma_method, range_method, multiplier};

  const auto kc_middle = kc_method(asset_data);
  const auto expected_middle = ma_method(asset_data);
  ASSERT_EQ(kc_middle.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(kc_middle[0], expected_middle[0]);
  EXPECT_DOUBLE_EQ(kc_middle[1], expected_middle[1]);
  EXPECT_DOUBLE_EQ(kc_middle[2], expected_middle[2]);
  EXPECT_DOUBLE_EQ(kc_middle[3], expected_middle[3]);
  EXPECT_DOUBLE_EQ(kc_middle[4], expected_middle[4]);
  EXPECT_DOUBLE_EQ(kc_middle[5], expected_middle[5]);
  EXPECT_TRUE(std::isnan(kc_middle[6]) && std::isnan(expected_middle[6]));
  EXPECT_TRUE(std::isnan(kc_middle[7]) && std::isnan(expected_middle[7]));
  EXPECT_TRUE(std::isnan(kc_middle[8]) && std::isnan(expected_middle[8]));
  EXPECT_TRUE(std::isnan(kc_middle[9]) && std::isnan(expected_middle[9]));

  kc_method.output(KcOutput::upper);
  const auto kc_upper = kc_method(asset_data);
  ASSERT_EQ(kc_upper.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(kc_upper[0], 894.18784000000005);
  EXPECT_DOUBLE_EQ(kc_upper[1], 901.48479999999995);
  EXPECT_DOUBLE_EQ(kc_upper[2], 901.85599999999999);
  EXPECT_DOUBLE_EQ(kc_upper[3], 895.32000000000005);
  EXPECT_DOUBLE_EQ(kc_upper[4], 892.39999999999998);
  EXPECT_DOUBLE_EQ(kc_upper[5], 900.0);
  EXPECT_TRUE(std::isnan(kc_upper[6]));
  EXPECT_TRUE(std::isnan(kc_upper[7]));
  EXPECT_TRUE(std::isnan(kc_upper[8]));
  EXPECT_TRUE(std::isnan(kc_upper[9]));

  kc_method.output(KcOutput::lower);
  const auto kc_lower = kc_method(asset_data);
  ASSERT_EQ(kc_lower.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(kc_lower[0], 829.81215999999995);
  EXPECT_DOUBLE_EQ(kc_lower[1], 828.51520000000005);
  EXPECT_DOUBLE_EQ(kc_lower[2], 818.14400000000001);
  EXPECT_DOUBLE_EQ(kc_lower[3], 800.67999999999995);
  EXPECT_DOUBLE_EQ(kc_lower[4], 791.60000000000002);
  EXPECT_DOUBLE_EQ(kc_lower[5], 784.0);
  EXPECT_TRUE(std::isnan(kc_lower[6]));
  EXPECT_TRUE(std::isnan(kc_lower[7]));
  EXPECT_TRUE(std::isnan(kc_lower[8]));
  EXPECT_TRUE(std::isnan(kc_lower[9]));
}

TEST(KcMethodTest, middle_EMA_range_ATR_with_offset)
{
  const auto data_method = DataMethod{"close"};
  const auto ma_method = EmaMethod{5, data_method, 0};
  const auto range_method = AtrMethod{4};
  const auto multiplier = 2.0;
  const auto offset = 1;
  auto kc_method =
   KcMethod{KcOutput::middle, ma_method, range_method, multiplier, offset};

  const auto asset_data = pludux::AssetHistory{
   {"high", {865, 865, 875, 880, 875, 875, 840, 840, 875, 925}},
   {"low", {850, 850, 855, 845, 855, 820, 800, 800, 830, 815}},
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto kc_middle = kc_method(asset_data);
  const auto expected_middle = ma_method(asset_data);
  ASSERT_EQ(kc_middle.size(), asset_data.size() - offset);
  EXPECT_DOUBLE_EQ(kc_middle[0], expected_middle[1]);
  EXPECT_DOUBLE_EQ(kc_middle[1], expected_middle[2]);
  EXPECT_DOUBLE_EQ(kc_middle[2], expected_middle[3]);
  EXPECT_DOUBLE_EQ(kc_middle[3], expected_middle[4]);
  EXPECT_DOUBLE_EQ(kc_middle[4], expected_middle[5]);
  EXPECT_TRUE(std::isnan(kc_middle[5]));
  EXPECT_TRUE(std::isnan(kc_middle[6]));
  EXPECT_TRUE(std::isnan(kc_middle[7]));
  EXPECT_TRUE(std::isnan(kc_middle[8]));
  EXPECT_TRUE(std::isnan(kc_middle[9]));

  kc_method.output(KcOutput::upper);
  const auto kc_upper = kc_method(asset_data);
  ASSERT_EQ(kc_upper.size(), asset_data.size() - offset);
  EXPECT_DOUBLE_EQ(kc_upper[0], 923.57289858217598);
  EXPECT_DOUBLE_EQ(kc_upper[1], 934.41818576388891);
  EXPECT_DOUBLE_EQ(kc_upper[2], 945.37239583333337);
  EXPECT_DOUBLE_EQ(kc_upper[3], 949.71875);
  EXPECT_DOUBLE_EQ(kc_upper[4], 957.625);
  EXPECT_TRUE(std::isnan(kc_upper[5]));
  EXPECT_TRUE(std::isnan(kc_upper[6]));
  EXPECT_TRUE(std::isnan(kc_upper[7]));
  EXPECT_TRUE(std::isnan(kc_upper[8]));
  EXPECT_TRUE(std::isnan(kc_upper[9]));

  kc_method.output(KcOutput::lower);
  kc_method.offset(2);
  const auto kc_lower = kc_method(asset_data);
  ASSERT_EQ(kc_lower.size(), asset_data.size() - 2);
  EXPECT_DOUBLE_EQ(kc_lower[0], 779.35959201388891);
  EXPECT_DOUBLE_EQ(kc_lower[1], 765.29427083333337);
  EXPECT_DOUBLE_EQ(kc_lower[2], 756.28125);
  EXPECT_DOUBLE_EQ(kc_lower[3], 726.375);
  EXPECT_TRUE(std::isnan(kc_lower[4]));
  EXPECT_TRUE(std::isnan(kc_lower[5]));
  EXPECT_TRUE(std::isnan(kc_lower[6]));
  EXPECT_TRUE(std::isnan(kc_lower[7]));
  EXPECT_TRUE(std::isnan(kc_lower[8]));
  EXPECT_TRUE(std::isnan(kc_lower[9]));
}