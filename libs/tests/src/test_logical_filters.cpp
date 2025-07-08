import pludux.asset_history;
import pludux.asset_snapshot;
import pludux.screener;
import pludux.series;

#include <gtest/gtest.h>
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

TEST(BooleanFilterTest, EqualityAndFilter)
{
  const auto true_filter = TrueFilter{};
  const auto filter1 = AndFilter{true_filter, true_filter};
  const auto filter2 = AndFilter{true_filter, true_filter};

  EXPECT_TRUE(filter1 == filter2);
  EXPECT_FALSE(filter1 != filter2);
  EXPECT_EQ(filter1, filter2);
}

TEST(BooleanFilterTest, NotEqualAndFilter)
{
  const auto true_filter = TrueFilter{};
  const auto false_filter = FalseFilter{};
  const auto filter1 = AndFilter{true_filter, false_filter};
  const auto filter2 = AndFilter{false_filter, true_filter};

  EXPECT_TRUE(filter1 != filter2);
  EXPECT_FALSE(filter1 == filter2);
  EXPECT_NE(filter1, filter2);
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

TEST(BooleanFilterTest, EqualityOrFilter)
{
  const auto true_filter = TrueFilter{};
  const auto filter1 = OrFilter{true_filter, true_filter};
  const auto filter2 = OrFilter{true_filter, true_filter};

  EXPECT_TRUE(filter1 == filter2);
  EXPECT_FALSE(filter1 != filter2);
  EXPECT_EQ(filter1, filter2);
}

TEST(BooleanFilterTest, NotEqualOrFilter)
{
  const auto true_filter = TrueFilter{};
  const auto false_filter = FalseFilter{};
  const auto filter1 = OrFilter{true_filter, false_filter};
  const auto filter2 = OrFilter{false_filter, true_filter};

  EXPECT_TRUE(filter1 != filter2);
  EXPECT_FALSE(filter1 == filter2);
  EXPECT_NE(filter1, filter2);
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

TEST(BooleanFilterTest, EqualityNotFilter)
{
  const auto true_filter = TrueFilter{};
  const auto not_filter1 = NotFilter{true_filter};
  const auto not_filter2 = NotFilter{true_filter};

  EXPECT_TRUE(not_filter1 == not_filter2);
  EXPECT_FALSE(not_filter1 != not_filter2);
  EXPECT_EQ(not_filter1, not_filter2);
}

TEST(BooleanFilterTest, NotEqualNotFilter)
{
  const auto true_filter = TrueFilter{};
  const auto false_filter = FalseFilter{};
  const auto not_filter1 = NotFilter{true_filter};
  const auto not_filter2 = NotFilter{false_filter};

  EXPECT_TRUE(not_filter1 != not_filter2);
  EXPECT_FALSE(not_filter1 == not_filter2);
  EXPECT_NE(not_filter1, not_filter2);
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

TEST(BooleanFilterTest, EqualityXorFilter)
{
  const auto true_filter = TrueFilter{};
  const auto filter1 = XorFilter{true_filter, true_filter};
  const auto filter2 = XorFilter{true_filter, true_filter};

  EXPECT_TRUE(filter1 == filter2);
  EXPECT_FALSE(filter1 != filter2);
  EXPECT_EQ(filter1, filter2);
}

TEST(BooleanFilterTest, NotEqualXorFilter)
{
  const auto true_filter = TrueFilter{};
  const auto false_filter = FalseFilter{};
  const auto filter1 = XorFilter{true_filter, false_filter};
  const auto filter2 = XorFilter{false_filter, true_filter};

  EXPECT_TRUE(filter1 != filter2);
  EXPECT_FALSE(filter1 == filter2);
  EXPECT_NE(filter1, filter2);
}