#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/abs_diff_method.hpp>
#include <pludux/screener/data_method.hpp>
#include <pludux/screener/value_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;
using pludux::AssetSnapshot;

TEST(AbsDiffMethodTest, RunMethod)
{
  const auto operand1_method = ValueMethod{20};
  const auto operand2_method = DataMethod{"close"};
  const auto abs_diff_method = AbsDiffMethod{operand1_method, operand2_method};
  const auto asset_data = pludux::AssetHistory{{"close", {10, 15, 20, 25, 30}}};

  const auto result = abs_diff_method(asset_data);

  ASSERT_EQ(result.size(), asset_data.size());
  EXPECT_DOUBLE_EQ(result[0], 10);
  EXPECT_DOUBLE_EQ(result[1], 5);
  EXPECT_DOUBLE_EQ(result[2], 0);
  EXPECT_DOUBLE_EQ(result[3], 5);
  EXPECT_DOUBLE_EQ(result[4], 10);
}
