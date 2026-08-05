#include <gtest/gtest.h>

import pludux.backtest;

namespace pludux::backtest::tests {
namespace {

auto percentage_fee(double percentage,
                    BrokerFee::FeeTrigger trigger = BrokerFee::FeeTrigger::All)
 -> BrokerFee
{
  return BrokerFee{"Percentage",
                   BrokerFee::FeeType::PercentageNotional,
                   BrokerFee::FeePosition::LongAndShort,
                   trigger,
                   percentage};
}

auto fixed_fee(double amount,
               BrokerFee::FeeTrigger trigger = BrokerFee::FeeTrigger::All)
 -> BrokerFee
{
  return BrokerFee{"Fixed",
                   BrokerFee::FeeType::Fixed,
                   BrokerFee::FeePosition::LongAndShort,
                   trigger,
                   amount};
}

TEST(EntryOrderSizing, NearestQuantityUsesMarketStepAndMinimum)
{
  const auto market = Market{"Whole units", 1.0, 1.0};
  const auto broker = Broker{"No fees"};

  const auto rounded = size_entry_order(
   EntryOrderSizingRequest{2.6, 100.0, NearestQuantityConstraint{}},
   market,
   broker);
  ASSERT_TRUE(rounded);
  EXPECT_DOUBLE_EQ(rounded->entry.position_size(), 3.0);

  const auto raised = size_entry_order(
   EntryOrderSizingRequest{0.4, 100.0, NearestQuantityConstraint{}},
   market,
   broker);
  ASSERT_TRUE(raised);
  EXPECT_DOUBLE_EQ(raised->entry.position_size(), 1.0);
}

TEST(EntryOrderSizing, MaximumQuantityNeverRoundsUp)
{
  const auto result = size_entry_order(
   EntryOrderSizingRequest{2.6, 100.0, MaximumQuantityConstraint{}},
   Market{"Whole units", 1.0, 1.0},
   Broker{"No fees"});

  ASSERT_TRUE(result);
  EXPECT_DOUBLE_EQ(result->entry.position_size(), 2.0);
  EXPECT_TRUE(result->limited);
}

TEST(EntryOrderSizing, EntryBudgetIncludesPercentageFee)
{
  const auto result = size_entry_order(
   EntryOrderSizingRequest{10.0, 100.0, EntryCostBudgetConstraint{1'000.0}},
   Market{"Tenths", 0.1, 0.1},
   Broker{"Percentage fee", {percentage_fee(1.0)}});

  ASSERT_TRUE(result);
  EXPECT_DOUBLE_EQ(result->entry.position_size(), 9.9);
  EXPECT_NEAR(result->entry_fee, 9.9, 1e-12);
  EXPECT_NEAR(result->entry_cost, 999.9, 1e-12);
}

TEST(EntryOrderSizing, EntryBudgetIncludesFixedFee)
{
  const auto result = size_entry_order(
   EntryOrderSizingRequest{10.0, 100.0, EntryCostBudgetConstraint{1'000.0}},
   Market{"Whole units", 1.0, 1.0},
   Broker{"Fixed fee", {fixed_fee(25.0, BrokerFee::FeeTrigger::Entry)}});

  ASSERT_TRUE(result);
  EXPECT_DOUBLE_EQ(result->entry.position_size(), 9.0);
  EXPECT_DOUBLE_EQ(result->entry_fee, 25.0);
  EXPECT_DOUBLE_EQ(result->entry_cost, 925.0);
}

TEST(EntryOrderSizing, EntryFeeTriggerUsesOrderDirection)
{
  const auto broker =
   Broker{"Buy fee", {fixed_fee(10.0, BrokerFee::FeeTrigger::Buy)}};
  const auto market = Market{"Whole units", 1.0, 1.0};

  const auto long_order = size_entry_order(
   EntryOrderSizingRequest{10.0, 100.0, EntryCostBudgetConstraint{1'000.0}},
   market,
   broker);
  const auto short_order = size_entry_order(
   EntryOrderSizingRequest{-10.0, 100.0, EntryCostBudgetConstraint{1'000.0}},
   market,
   broker);

  ASSERT_TRUE(long_order);
  ASSERT_TRUE(short_order);
  EXPECT_DOUBLE_EQ(long_order->entry.position_size(), 9.0);
  EXPECT_DOUBLE_EQ(short_order->entry.position_size(), -10.0);
}

TEST(EntryOrderSizing, RiskBudgetIncludesRoundTripFeesForLongAndShort)
{
  const auto broker = Broker{"Round-trip fee", {fixed_fee(5.0)}};
  const auto market = Market{"Whole units", 1.0, 1.0};

  const auto long_order = size_entry_order(
   EntryOrderSizingRequest{20.0, 100.0, RiskBudgetConstraint{100.0, 95.0}},
   market,
   broker);
  const auto short_order = size_entry_order(
   EntryOrderSizingRequest{-20.0, 100.0, RiskBudgetConstraint{100.0, 105.0}},
   market,
   broker);

  ASSERT_TRUE(long_order);
  ASSERT_TRUE(short_order);
  EXPECT_DOUBLE_EQ(long_order->entry.position_size(), 18.0);
  EXPECT_DOUBLE_EQ(short_order->entry.position_size(), -18.0);
  EXPECT_DOUBLE_EQ(long_order->entry_fee, 5.0);
  EXPECT_DOUBLE_EQ(long_order->estimated_exit_fee, 5.0);
  EXPECT_DOUBLE_EQ(*long_order->estimated_loss, 100.0);
  EXPECT_DOUBLE_EQ(*short_order->estimated_loss, 100.0);
}

TEST(EntryOrderSizing, ReturnsNoOrderWhenMinimumExceedsBudget)
{
  const auto result = size_entry_order(
   EntryOrderSizingRequest{10.0, 100.0, EntryCostBudgetConstraint{150.0}},
   Market{"Two minimum", 2.0, 1.0},
   Broker{"No fees"});

  EXPECT_FALSE(result);
}

TEST(EntryOrderSizing, ReproducesWholeUnitEquityAllocationCase)
{
  const auto result = size_entry_order(
   EntryOrderSizingRequest{
    886'925.0 / 8'375.0, 8'375.0, EntryCostBudgetConstraint{886'925.0}},
   Market{"Whole units", 1.0, 1.0},
   Broker{"No fees"});

  ASSERT_TRUE(result);
  EXPECT_DOUBLE_EQ(result->entry.position_size(), 105.0);
  EXPECT_DOUBLE_EQ(result->entry_cost, 879'375.0);
}

} // namespace
} // namespace pludux::backtest::tests
