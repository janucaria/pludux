#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;
using namespace pludux::screener;

TEST(BbMethodTest, ConstructorInitialization)
{
  {
    auto bb_method = BbMethod{};

    EXPECT_EQ(bb_method.ma_type(), BbMaType::Sma);
    EXPECT_EQ(bb_method.ma_source(), CloseMethod{});
    EXPECT_EQ(bb_method.period(), 20);
    EXPECT_EQ(bb_method.stddev(), 1.5);
  }
  {
    auto bb_method = BbMethod{10, 2.0};

    EXPECT_EQ(bb_method.ma_type(), BbMaType::Sma);
    EXPECT_EQ(bb_method.ma_source(), CloseMethod{});
    EXPECT_EQ(bb_method.period(), 10);
    EXPECT_EQ(bb_method.stddev(), 2.0);
  }
  {
    const auto bb_method =
     BbMethod{BbMaType::Ema, DataMethod{"close"}, 10, 2.5};

    EXPECT_EQ(bb_method.ma_type(), BbMaType::Ema);
    EXPECT_EQ(bb_method.ma_source(), DataMethod{"close"});
    EXPECT_EQ(bb_method.period(), 10);
    EXPECT_EQ(bb_method.stddev(), 2.5);
  }
}

TEST(BbMethodTest, RunAllMethod)
{
  const auto asset_data = pludux::AssetHistory{
   {"Close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  const auto middle_band = BbMethod{5, 2.0};
  EXPECT_DOUBLE_EQ(middle_band(asset_snapshot[0], context), 862);
  EXPECT_DOUBLE_EQ(middle_band(asset_snapshot[1], context), 865);
  EXPECT_DOUBLE_EQ(middle_band(asset_snapshot[2], context), 860);
  EXPECT_DOUBLE_EQ(middle_band(asset_snapshot[3], context), 848);
  EXPECT_DOUBLE_EQ(middle_band(asset_snapshot[4], context), 842);
  EXPECT_DOUBLE_EQ(middle_band(asset_snapshot[5], context), 842);
  EXPECT_TRUE(std::isnan(middle_band(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(middle_band(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(middle_band(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(middle_band(asset_snapshot[9], context)));

  const auto upper_band =
   SelectOutputMethod{middle_band, MethodOutput::UpperBand};
  EXPECT_DOUBLE_EQ(upper_band(asset_snapshot[0], context), 875.56465996625059);
  EXPECT_DOUBLE_EQ(upper_band(asset_snapshot[1], context), 877.64911064067348);
  EXPECT_DOUBLE_EQ(upper_band(asset_snapshot[2], context), 887.5680975041804);
  EXPECT_DOUBLE_EQ(upper_band(asset_snapshot[3], context), 903.35341001239215);
  EXPECT_DOUBLE_EQ(upper_band(asset_snapshot[4], context), 897.35341001239215);
  EXPECT_DOUBLE_EQ(upper_band(asset_snapshot[5], context), 897.35341001239215);
  EXPECT_TRUE(std::isnan(upper_band(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(upper_band(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(upper_band(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(upper_band(asset_snapshot[9], context)));

  const auto lower_band =
   SelectOutputMethod{middle_band, MethodOutput::LowerBand};
  EXPECT_DOUBLE_EQ(lower_band(asset_snapshot[0], context), 848.43534003374941);
  EXPECT_DOUBLE_EQ(lower_band(asset_snapshot[1], context), 852.35088935932652);
  EXPECT_DOUBLE_EQ(lower_band(asset_snapshot[2], context), 832.4319024958196);
  EXPECT_DOUBLE_EQ(lower_band(asset_snapshot[3], context), 792.64658998760785);
  EXPECT_DOUBLE_EQ(lower_band(asset_snapshot[4], context), 786.64658998760785);
  EXPECT_DOUBLE_EQ(lower_band(asset_snapshot[5], context), 786.64658998760785);
  EXPECT_TRUE(std::isnan(lower_band(asset_snapshot[6], context)));
  EXPECT_TRUE(std::isnan(lower_band(asset_snapshot[7], context)));
  EXPECT_TRUE(std::isnan(lower_band(asset_snapshot[8], context)));
  EXPECT_TRUE(std::isnan(lower_band(asset_snapshot[9], context)));
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
  const auto bb_method1 = BbMethod{BbMaType::Sma, DataMethod{"close"}, 10, 2.0};
  const auto bb_method2 = BbMethod{BbMaType::Sma, DataMethod{"close"}, 10, 1.0};
  const auto bb_method3 = BbMethod{BbMaType::Sma, DataMethod{"open"}, 10, 1.0};

  EXPECT_TRUE(bb_method1 != bb_method2);
  EXPECT_NE(bb_method1, bb_method2);
  EXPECT_TRUE(bb_method1 != bb_method3);
  EXPECT_NE(bb_method1, bb_method3);
  EXPECT_TRUE(bb_method2 != bb_method3);
  EXPECT_NE(bb_method2, bb_method3);
}
