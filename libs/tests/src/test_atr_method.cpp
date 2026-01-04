#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

TEST(AtrMethodTest, ConstructorInitialization)
{
  {
    auto atr_method = AtrMethod{14};

    EXPECT_EQ(atr_method.period(), 14);
    EXPECT_EQ(atr_method.ma_smoothing_type(), MaMethodType::Rma);
  }
  {
    auto atr_method = AtrMethod{MaMethodType::Ema, 14};

    EXPECT_EQ(atr_method.period(), 14);
    EXPECT_EQ(atr_method.ma_smoothing_type(), MaMethodType::Ema);
  }
}

TEST(AtrMethodTest, RunAllMethod)
{
  const auto asset_data =
   AssetHistory{{"High", {865, 865, 875, 880, 875, 875, 840, 840, 875, 925}},
                {"Low", {850, 850, 855, 845, 855, 820, 800, 800, 830, 815}},
                {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  const auto atr_method = AtrMethod{5};

  EXPECT_DOUBLE_EQ(atr_method(asset_snapshot[0], context), 32.187840000000008);
  EXPECT_DOUBLE_EQ(atr_method(asset_snapshot[1], context), 36.484800000000007);
  EXPECT_DOUBLE_EQ(atr_method(asset_snapshot[2], context), 41.856000000000009);
  EXPECT_DOUBLE_EQ(atr_method(asset_snapshot[3], context), 47.320000000000007);
  EXPECT_DOUBLE_EQ(atr_method(asset_snapshot[4], context), 50.400000000000006);
  EXPECT_DOUBLE_EQ(atr_method(asset_snapshot[5], context), 58);
  EXPECT_TRUE(std::isnan(atr_method(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(atr_method(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(atr_method(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(atr_method(asset_snapshot[9], context)));
}

TEST(AtrMethodTest, EqualityOperator)
{
  const auto atr_method1 = AtrMethod{14};
  const auto atr_method2 = AtrMethod{14};

  EXPECT_TRUE(atr_method1 == atr_method2);
  EXPECT_EQ(atr_method1, atr_method2);
}

TEST(AtrMethodTest, NotEqualOperator)
{
  const auto atr_method1 = AtrMethod{MaMethodType::Ema, 14};
  const auto atr_method2 = AtrMethod{MaMethodType::Ema, 20};
  const auto atr_method3 = AtrMethod{MaMethodType::Rma, 20};

  EXPECT_TRUE(atr_method1 != atr_method2);
  EXPECT_NE(atr_method1, atr_method2);
  EXPECT_TRUE(atr_method1 != atr_method3);
  EXPECT_NE(atr_method1, atr_method3);
  EXPECT_TRUE(atr_method2 != atr_method3);
  EXPECT_NE(atr_method2, atr_method3);
}
