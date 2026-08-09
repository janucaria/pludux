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
   EntryOrderSizingRequest{.requested_quantity = 2.6,
                           .entry_price = 100.0,
                           .constraint = NearestQuantityConstraint{},
                           .risk_distance = 5.0},
   market,
   broker);
  ASSERT_TRUE(rounded);
  EXPECT_DOUBLE_EQ(rounded->entry().position_size(), 3.0);

  const auto raised = size_entry_order(
   EntryOrderSizingRequest{.requested_quantity = 0.4,
                           .entry_price = 100.0,
                           .constraint = NearestQuantityConstraint{},
                           .risk_distance = 5.0},
   market,
   broker);
  ASSERT_TRUE(raised);
  EXPECT_DOUBLE_EQ(raised->entry().position_size(), 1.0);
}

TEST(EntryOrderSizing, MaximumQuantityNeverRoundsUp)
{
  const auto result = size_entry_order(
   EntryOrderSizingRequest{.requested_quantity = 2.6,
                           .entry_price = 100.0,
                           .constraint = MaximumQuantityConstraint{},
                           .risk_distance = 5.0},
   Market{"Whole units", 1.0, 1.0},
   Broker{"No fees"});

  ASSERT_TRUE(result);
  EXPECT_DOUBLE_EQ(result->entry().position_size(), 2.0);
  EXPECT_LT(result->requested_quantity(), 2.6);
}

TEST(EntryOrderSizing, EntryBudgetIncludesPercentageFee)
{
  const auto result = size_entry_order(
   EntryOrderSizingRequest{.requested_quantity = 10.0,
                           .entry_price = 100.0,
                           .constraint = EntryCostBudgetConstraint{1'000.0},
                           .risk_distance = 5.0},
   Market{"Tenths", 0.1, 0.1},
   Broker{"Percentage fee", {percentage_fee(1.0)}});

  ASSERT_TRUE(result);
  EXPECT_DOUBLE_EQ(result->entry().position_size(), 9.9);
  EXPECT_NEAR(result->estimated_entry_fee(), 9.9, 1e-12);
  EXPECT_NEAR(result->requested_cost(), 999.9, 1e-12);
}

TEST(EntryOrderSizing, EntryBudgetIncludesFixedFee)
{
  const auto result = size_entry_order(
   EntryOrderSizingRequest{.requested_quantity = 10.0,
                           .entry_price = 100.0,
                           .constraint = EntryCostBudgetConstraint{1'000.0},
                           .risk_distance = 5.0},
   Market{"Whole units", 1.0, 1.0},
   Broker{"Fixed fee", {fixed_fee(25.0, BrokerFee::FeeTrigger::Entry)}});

  ASSERT_TRUE(result);
  EXPECT_DOUBLE_EQ(result->entry().position_size(), 9.0);
  EXPECT_DOUBLE_EQ(result->estimated_entry_fee(), 25.0);
  EXPECT_DOUBLE_EQ(result->requested_cost(), 925.0);
}

TEST(EntryOrderSizing, EntryFeeTriggerUsesOrderDirection)
{
  const auto broker =
   Broker{"Buy fee", {fixed_fee(10.0, BrokerFee::FeeTrigger::Buy)}};
  const auto market = Market{"Whole units", 1.0, 1.0};

  const auto long_order = size_entry_order(
   EntryOrderSizingRequest{.requested_quantity = 10.0,
                           .entry_price = 100.0,
                           .constraint = EntryCostBudgetConstraint{1'000.0},
                           .risk_distance = 5.0},
   market,
   broker);
  const auto short_order = size_entry_order(
   EntryOrderSizingRequest{.requested_quantity = -10.0,
                           .entry_price = 100.0,
                           .constraint = EntryCostBudgetConstraint{1'000.0},
                           .risk_distance = 5.0},
   market,
   broker);

  ASSERT_TRUE(long_order);
  ASSERT_TRUE(short_order);
  EXPECT_DOUBLE_EQ(long_order->entry().position_size(), 9.0);
  EXPECT_DOUBLE_EQ(short_order->entry().position_size(), -10.0);
}

TEST(EntryOrderSizing, RiskBudgetIncludesRoundTripFeesForLongAndShort)
{
  const auto broker = Broker{"Round-trip fee", {fixed_fee(5.0)}};
  const auto market = Market{"Whole units", 1.0, 1.0};

  const auto long_order = size_entry_order(
   EntryOrderSizingRequest{.requested_quantity = 20.0,
                           .entry_price = 100.0,
                           .constraint = RiskBudgetConstraint{100.0, 95.0},
                           .risk_distance = 5.0},
   market,
   broker);
  const auto short_order = size_entry_order(
   EntryOrderSizingRequest{.requested_quantity = -20.0,
                           .entry_price = 100.0,
                           .constraint = RiskBudgetConstraint{100.0, 105.0},
                           .risk_distance = 5.0},
   market,
   broker);

  ASSERT_TRUE(long_order);
  ASSERT_TRUE(short_order);
  EXPECT_DOUBLE_EQ(long_order->entry().position_size(), 18.0);
  EXPECT_DOUBLE_EQ(short_order->entry().position_size(), -18.0);
  EXPECT_DOUBLE_EQ(long_order->estimated_entry_fee(), 5.0);
  EXPECT_DOUBLE_EQ(long_order->estimated_exit_fee(), 5.0);
  EXPECT_DOUBLE_EQ(long_order->requested_risk_with_fees(), 100.0);
  EXPECT_DOUBLE_EQ(short_order->requested_risk_with_fees(), 100.0);
}

TEST(EntryOrderSizing, ReturnsNoOrderWhenMinimumExceedsBudget)
{
  const auto result = size_entry_order(
   EntryOrderSizingRequest{.requested_quantity = 10.0,
                           .entry_price = 100.0,
                           .constraint = EntryCostBudgetConstraint{150.0},
                           .risk_distance = 5.0},
   Market{"Two minimum", 2.0, 1.0},
   Broker{"No fees"});

  EXPECT_FALSE(result);
}

TEST(EntryOrderSizing, ReproducesWholeUnitEquityAllocationCase)
{
  const auto result = size_entry_order(
   EntryOrderSizingRequest{.requested_quantity = 886'925.0 / 8'375.0,
                           .entry_price = 8'375.0,
                           .constraint = EntryCostBudgetConstraint{886'925.0},
                           .risk_distance = 100.0},
   Market{"Whole units", 1.0, 1.0},
   Broker{"No fees"});

  ASSERT_TRUE(result);
  EXPECT_DOUBLE_EQ(result->entry().position_size(), 105.0);
  EXPECT_DOUBLE_EQ(result->requested_cost(), 879'375.0);
}

TEST(EntryOrderSizing, PreservesEveryPreOrderSizingStage)
{
  const auto result = size_entry_order(
   EntryOrderSizingRequest{.requested_quantity = 8.4,
                           .entry_price = 100.0,
                           .constraint = MaximumQuantityConstraint{},
                           .pyramiding = false,
                           .raw_requested_quantity = 10.0,
                           .raw_requested_limit = 1'000.0,
                           .drawdown_adjusted_quantity = 8.4,
                           .drawdown_adjusted_limit = 800.0,
                           .risk_distance = 5.0},
   Market{"Whole units", 1.0, 1.0},
   Broker{"No fees"});

  ASSERT_TRUE(result);
  EXPECT_EQ(result->raw_requested_quantity(), 10.0);
  EXPECT_EQ(result->raw_requested_limit(), 1'000.0);
  EXPECT_EQ(result->drawdown_adjusted_quantity(), 8.4);
  EXPECT_EQ(result->drawdown_adjusted_limit(), 800.0);
  EXPECT_DOUBLE_EQ(result->requested_quantity(), 8.0);
  EXPECT_DOUBLE_EQ(result->requested_notional(), 800.0);
  EXPECT_DOUBLE_EQ(result->requested_price_risk(), 40.0);
}

} // namespace
} // namespace pludux::backtest::tests
