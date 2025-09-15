#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(SmaMethodTest, RunOneMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field};
  const auto period = 5;
  const auto sma_method = SmaMethod{period, field_method};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = sma_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, 862);
}

TEST(SmaMethodTest, RunAllMethod)
{
  const auto field = "close";
  const auto field_method = DataMethod{field};
  const auto period = 5;
  const auto sma_method = SmaMethod{period, field_method};
  const auto asset_data = pludux::AssetHistory{
   {"close", {855, 860, 860, 860, 875, 870, 835, 800, 830, 875}}};

  const auto result = sma_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(result[0], 862);
  EXPECT_DOUBLE_EQ(result[1], 865);
  EXPECT_DOUBLE_EQ(result[2], 860);
  EXPECT_DOUBLE_EQ(result[3], 848);
  EXPECT_DOUBLE_EQ(result[4], 842);
  EXPECT_DOUBLE_EQ(result[5], 842);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}

TEST(SmaMethodTest, EqualityOperator)
{
  const auto field_method = DataMethod{"close"};
  const auto sma_method1 = SmaMethod{5, field_method};
  const auto sma_method2 = SmaMethod{5, field_method};

  EXPECT_TRUE(sma_method1 == sma_method2);
  EXPECT_EQ(sma_method1, sma_method2);
}

TEST(SmaMethodTest, NotEqualOperator)
{
  const auto sma_method1 = SmaMethod{1, DataMethod{"close"}};
  const auto sma_method2 = SmaMethod{1, DataMethod{"open"}};
  const auto sma_method3 = SmaMethod{2, DataMethod{"close"}};

  EXPECT_NE(sma_method1, sma_method2);
  EXPECT_NE(sma_method1, sma_method3);
  EXPECT_NE(sma_method2, sma_method3);
}
