#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/boolean_filter.hpp>
#include <pludux/screener/logical_filter.hpp>

using namespace pludux::screener;
using pludux::AssetHistory;

TEST(BooleanFilterTest, AndFilter)
{
  const auto true_filter = TrueFilter{};
  const auto false_filter = FalseFilter{};
  const auto asset_data = AssetHistory{};

  const auto true_and_true = AndFilter{true_filter, true_filter};
  EXPECT_TRUE(true_and_true(asset_data));

  const auto true_and_false = AndFilter{true_filter, false_filter};
  EXPECT_FALSE(true_and_false(asset_data));

  const auto false_and_true = AndFilter{false_filter, true_filter};
  EXPECT_FALSE(false_and_true(asset_data));

  const auto false_and_false = AndFilter{false_filter, false_filter};
  EXPECT_FALSE(false_and_false(asset_data));
}

TEST(BooleanFilterTest, OrFilter)
{
  const auto true_filter = TrueFilter{};
  const auto false_filter = FalseFilter{};
  const auto asset_data = AssetHistory{};

  const auto true_or_true = OrFilter{true_filter, true_filter};
  EXPECT_TRUE(true_or_true(asset_data));

  const auto true_or_false = OrFilter{true_filter, false_filter};
  EXPECT_TRUE(true_or_false(asset_data));

  const auto false_or_true = OrFilter{false_filter, true_filter};
  EXPECT_TRUE(false_or_true(asset_data));

  const auto false_or_false = OrFilter{false_filter, false_filter};
  EXPECT_FALSE(false_or_false(asset_data));
}

TEST(BooleanFilterTest, NotFilter)
{
  const auto true_filter = TrueFilter{};
  const auto false_filter = FalseFilter{};
  const auto asset_data = AssetHistory{};

  const auto not_true = NotFilter{true_filter};
  EXPECT_FALSE(not_true(asset_data));

  const auto not_false = NotFilter{false_filter};
  EXPECT_TRUE(not_false(asset_data));
}

TEST(BooleanFilterTest, XorFilter)
{
  const auto true_filter = TrueFilter{};
  const auto false_filter = FalseFilter{};
  const auto asset_data = AssetHistory{};

  const auto true_xor_true = XorFilter{true_filter, true_filter};
  EXPECT_FALSE(true_xor_true(asset_data));

  const auto true_xor_false = XorFilter{true_filter, false_filter};
  EXPECT_TRUE(true_xor_false(asset_data));

  const auto false_xor_true = XorFilter{false_filter, true_filter};
  EXPECT_TRUE(false_xor_true(asset_data));

  const auto false_xor_false = XorFilter{false_filter, false_filter};
  EXPECT_FALSE(false_xor_false(asset_data));
}