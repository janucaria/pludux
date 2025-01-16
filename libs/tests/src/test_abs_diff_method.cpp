#include <gtest/gtest.h>
#include <pludux/asset_data_provider.hpp>
#include <pludux/screener/abs_diff_method.hpp>
#include <pludux/screener/value_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;
using pludux::Quote;

TEST(AbsDiffMethodTest, RunAllMethod)
{
  const auto operand1_method = ValueMethod{10.0};
  const auto operand2_method = ValueMethod{5.0};
  const auto abs_diff_method = AbsDiffMethod{operand1_method, operand2_method};
  const auto asset = pludux::Asset{"", std::vector<Quote>(5)};
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto result = abs_diff_method(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], 5.0);
  EXPECT_DOUBLE_EQ(result[1], 5.0);
  EXPECT_DOUBLE_EQ(result[2], 5.0);
  EXPECT_DOUBLE_EQ(result[3], 5.0);
  EXPECT_DOUBLE_EQ(result[4], 5.0);
}

TEST(AbsDiffMethodTest, RunAllMethodWithDifferentValues)
{
  const auto operand1_method = ValueMethod{20.0};
  const auto operand2_method = ValueMethod{15.0};
  const auto abs_diff_method = AbsDiffMethod{operand1_method, operand2_method};
  const auto asset = pludux::Asset{"", std::vector<Quote>(5)};
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto result = abs_diff_method(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], 5.0);
  EXPECT_DOUBLE_EQ(result[1], 5.0);
  EXPECT_DOUBLE_EQ(result[2], 5.0);
  EXPECT_DOUBLE_EQ(result[3], 5.0);
  EXPECT_DOUBLE_EQ(result[4], 5.0);
}

TEST(AbsDiffMethodTest, RunAllMethodWithZeroDifference)
{
  const auto operand1_method = ValueMethod{10.0};
  const auto operand2_method = ValueMethod{10.0};
  const auto abs_diff_method = AbsDiffMethod{operand1_method, operand2_method};
  const auto asset = pludux::Asset{"", std::vector<Quote>(5)};
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto result = abs_diff_method(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], 0.0);
  EXPECT_DOUBLE_EQ(result[1], 0.0);
  EXPECT_DOUBLE_EQ(result[2], 0.0);
  EXPECT_DOUBLE_EQ(result[3], 0.0);
  EXPECT_DOUBLE_EQ(result[4], 0.0);
}