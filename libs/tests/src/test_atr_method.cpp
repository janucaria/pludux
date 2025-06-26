#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/atr_method.hpp>
#include <pludux/screener/data_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;
using pludux::AssetSnapshot;

TEST(AtrMethodTest, RunOneMethod)
{
  const auto period = 5;
  const auto atr_method = AtrMethod{
   DataMethod{"high"}, DataMethod{"low"}, DataMethod{"close"}, period};
  const auto asset_data = pludux::AssetHistory{
   {"high", {865, 865, 875, 880, 875, 875, 840, 840, 875, 925}},
   {"low", {850, 850, 855, 845, 855, 820, 800, 800, 830, 815}},
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = atr_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, 32.187840000000008);
}

TEST(AtrMethodTest, RunAllMethod)
{
  const auto period = 5;
  const auto atr_method = AtrMethod{
   DataMethod{"high"}, DataMethod{"low"}, DataMethod{"close"}, period};
  const auto asset_data = pludux::AssetHistory{
   {"high", {865, 865, 875, 880, 875, 875, 840, 840, 875, 925}},
   {"low", {850, 850, 855, 845, 855, 820, 800, 800, 830, 815}},
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = atr_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(result[0], 32.187840000000008);
  EXPECT_DOUBLE_EQ(result[1], 36.484800000000007);
  EXPECT_DOUBLE_EQ(result[2], 41.856000000000009);
  EXPECT_DOUBLE_EQ(result[3], 47.320000000000007);
  EXPECT_DOUBLE_EQ(result[4], 50.400000000000006);
  EXPECT_DOUBLE_EQ(result[5], 58);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(AtrMethodTest, EqualityOperator)
{
  const auto operand1_method1 = DataMethod{"high"};
  const auto operand2_method1 = DataMethod{"low"};
  const auto operand3_method1 = DataMethod{"close"};
  const auto atr_method1 =
   AtrMethod{operand1_method1, operand2_method1, operand3_method1, 14};

  const auto operand1_method2 = DataMethod{"high"};
  const auto operand2_method2 = DataMethod{"low"};
  const auto operand3_method2 = DataMethod{"close"};
  const auto atr_method2 =
   AtrMethod{operand1_method2, operand2_method2, operand3_method2, 14};

  EXPECT_TRUE(atr_method1 == atr_method2);
  EXPECT_FALSE(atr_method1 != atr_method2);
  EXPECT_EQ(atr_method1, atr_method2);
}

TEST(AtrMethodTest, NotEqualOperator)
{
  const auto operand1_method1 = DataMethod{"high"};
  const auto operand2_method1 = DataMethod{"low"};
  const auto operand3_method1 = DataMethod{"close"};
  const auto atr_method1 =
   AtrMethod{operand1_method1, operand2_method1, operand3_method1, 14};

  const auto operand1_method2 = DataMethod{"high"};
  const auto operand2_method2 = DataMethod{"low"};
  const auto operand3_method2 = DataMethod{"close"};
  const auto atr_method2 =
   AtrMethod{operand1_method2, operand2_method2, operand3_method2, 20};

  EXPECT_TRUE(atr_method1 != atr_method2);
  EXPECT_FALSE(atr_method1 == atr_method2);
  EXPECT_NE(atr_method1, atr_method2);
}
