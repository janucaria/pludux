#include <gtest/gtest.h>

import pludux;

using namespace pludux;
using namespace pludux::screener;

TEST(BbMethodTest, RunAllMethod)
{
  const auto data_method = DataMethod{"close"};
  const auto period = 5;
  const auto std_dev = 2.0;
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  auto bb_method =
   BbMethod{BbMethod::MaType::ema, data_method, period, std_dev};
  auto bb_series = OutputByNameSeries{
   BbSeries{EmaSeries{data_method(asset_data), period}, std_dev},
   OutputName::MiddleBand};

  const auto bb_middle = bb_method(asset_data);
  const auto& expected_middle = bb_series;
  ASSERT_EQ(bb_middle.size(), expected_middle.size());
  EXPECT_DOUBLE_EQ(bb_middle[0], expected_middle[0]);
  EXPECT_DOUBLE_EQ(bb_middle[1], expected_middle[1]);
  EXPECT_DOUBLE_EQ(bb_middle[2], expected_middle[2]);
  EXPECT_DOUBLE_EQ(bb_middle[3], expected_middle[3]);
  EXPECT_DOUBLE_EQ(bb_middle[4], expected_middle[4]);
  EXPECT_DOUBLE_EQ(bb_middle[5], 842);
  EXPECT_TRUE(std::isnan(bb_middle[6]) && std::isnan(expected_middle[6]));
  EXPECT_TRUE(std::isnan(bb_middle[7]) && std::isnan(expected_middle[7]));
  EXPECT_TRUE(std::isnan(bb_middle[8]) && std::isnan(expected_middle[8]));
  EXPECT_TRUE(std::isnan(bb_middle[9]) && std::isnan(expected_middle[9]));

  bb_series.output_name(OutputName::UpperBand);
  const auto bb_upper =
   OutputByNameMethod{bb_method, OutputName::UpperBand}(asset_data);
  const auto& expected_upper = bb_series;
  ASSERT_EQ(bb_upper.size(), expected_upper.size());
  EXPECT_DOUBLE_EQ(bb_upper[0], expected_upper[0]);
  EXPECT_DOUBLE_EQ(bb_upper[1], expected_upper[1]);
  EXPECT_DOUBLE_EQ(bb_upper[2], expected_upper[2]);
  EXPECT_DOUBLE_EQ(bb_upper[3], expected_upper[3]);
  EXPECT_DOUBLE_EQ(bb_upper[4], expected_upper[4]);
  EXPECT_DOUBLE_EQ(bb_upper[5], expected_upper[5]);
  EXPECT_TRUE(std::isnan(bb_upper[6]) && std::isnan(expected_upper[6]));
  EXPECT_TRUE(std::isnan(bb_upper[7]) && std::isnan(expected_upper[7]));
  EXPECT_TRUE(std::isnan(bb_upper[8]) && std::isnan(expected_upper[8]));
  EXPECT_TRUE(std::isnan(bb_upper[9]) && std::isnan(expected_upper[9]));

  bb_series.output_name(OutputName::LowerBand);
  const auto bb_lower =
   OutputByNameMethod{bb_method, OutputName::LowerBand}(asset_data);
  const auto& expected_lower = bb_series;
  ASSERT_EQ(bb_lower.size(), expected_lower.size());
  EXPECT_DOUBLE_EQ(bb_lower[0], expected_lower[0]);
  EXPECT_DOUBLE_EQ(bb_lower[1], expected_lower[1]);
  EXPECT_DOUBLE_EQ(bb_lower[2], expected_lower[2]);
  EXPECT_DOUBLE_EQ(bb_lower[3], expected_lower[3]);
  EXPECT_DOUBLE_EQ(bb_lower[4], expected_lower[4]);
  EXPECT_DOUBLE_EQ(bb_lower[5], expected_lower[5]);
  EXPECT_TRUE(std::isnan(bb_lower[6]) && std::isnan(expected_lower[6]));
  EXPECT_TRUE(std::isnan(bb_lower[7]) && std::isnan(expected_lower[7]));
  EXPECT_TRUE(std::isnan(bb_lower[8]) && std::isnan(expected_lower[8]));
  EXPECT_TRUE(std::isnan(bb_lower[9]) && std::isnan(expected_lower[9]));
}

TEST(BbMethodTest, EqualityOperator)
{
  const auto input_method1 = DataMethod{"close"};
  const auto bb_method1 =
   BbMethod{BbMethod::MaType::sma, input_method1, 20, 2.0};

  const auto input_method2 = DataMethod{"close"};
  const auto bb_method2 =
   BbMethod{BbMethod::MaType::sma, input_method2, 20, 2.0};

  EXPECT_TRUE(bb_method1 == bb_method2);
  EXPECT_FALSE(bb_method1 != bb_method2);
  EXPECT_EQ(bb_method1, bb_method2);
}

TEST(BbMethodTest, NotEqualOperator)
{
  const auto input_method1 = DataMethod{"close"};
  const auto bb_method1 =
   BbMethod{BbMethod::MaType::sma, input_method1, 20, 2.0};

  const auto input_method2 = DataMethod{"open"};
  const auto bb_method2 =
   BbMethod{BbMethod::MaType::ema, input_method2, 20, 2.0};

  EXPECT_TRUE(bb_method1 != bb_method2);
  EXPECT_FALSE(bb_method1 == bb_method2);
  EXPECT_NE(bb_method1, bb_method2);
}
