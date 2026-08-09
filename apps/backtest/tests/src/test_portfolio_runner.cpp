#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

auto make_asset(std::string name, std::vector<double> timestamps) -> Asset
{
  const auto size = timestamps.size();
  const auto filled = [size](double value) {
    const auto values = std::vector<double>(size, value);
    return AssetData{values.begin(), values.end()};
  };
  const auto fields =
   std::array{std::pair{std::string{"Datetime"},
                        AssetData{timestamps.begin(), timestamps.end()}},
              std::pair{std::string{"Open"}, filled(100.0)},
              std::pair{std::string{"High"}, filled(100.0)},
              std::pair{std::string{"Low"}, filled(100.0)},
              std::pair{std::string{"Close"}, filled(100.0)},
              std::pair{std::string{"Volume"}, filled(0.0)}};
  return Asset{std::move(name), AssetHistory{fields.begin(), fields.end()}};
}

auto make_priced_asset(std::string name, double price) -> Asset
{
  const auto scalar = [price] { return AssetData{&price, &price + 1}; };
  const auto timestamp = 1.0;
  const auto volume = 0.0;
  const auto fields = std::array{
   std::pair{std::string{"Datetime"}, AssetData{&timestamp, &timestamp + 1}},
   std::pair{std::string{"Open"}, scalar()},
   std::pair{std::string{"High"}, scalar()},
   std::pair{std::string{"Low"}, scalar()},
   std::pair{std::string{"Close"}, scalar()},
   std::pair{std::string{"Volume"}, AssetData{&volume, &volume + 1}}};
  return Asset{std::move(name), AssetHistory{fields.begin(), fields.end()}};
}

auto make_runner(const Asset& asset,
                 const Market& market,
                 const Broker& broker,
                 const Profile& profile) -> BacktestRunner
{
  return BacktestRunner{asset,
                        market,
                        broker,
                        profile,
                        {},
                        BacktestRunner::PositionRule{},
                        BacktestRunner::PositionRule{},
                        1'000.0};
}

auto make_entry_runner(const Asset& asset,
                       const Market& market,
                       const Broker& broker,
                       const Profile& profile,
                       InsufficientCashPolicy cash_policy,
                       ErasedSeriesMethod<ExecutionFilterMethodContext>
                        execution_filter = BooleanMethod<true>{})
 -> BacktestRunner
{
  auto long_position = BacktestRunner::PositionRule{BooleanMethod<true>{},
                                                    {},
                                                    BooleanMethod<false>{},
                                                    0,
                                                    0,
                                                    ValueMethod{10.0},
                                                    {},
                                                    SignalTiming::CurrentClose};
  return BacktestRunner{asset,
                        market,
                        broker,
                        profile,
                        {},
                        std::move(long_position),
                        BacktestRunner::PositionRule{},
                        1'000.0,
                        0,
                        false,
                        NAN,
                        IntrabarPath::CandleDirection,
                        {},
                        std::move(execution_filter),
                        DrawdownAdjustment{},
                        cash_policy};
}

auto make_short_entry_runner(const Asset& asset,
                             const Market& market,
                             const Broker& broker,
                             const Profile& profile) -> BacktestRunner
{
  auto short_position =
   BacktestRunner::PositionRule{BooleanMethod<true>{},
                                {},
                                BooleanMethod<false>{},
                                0,
                                0,
                                ValueMethod{10.0},
                                {},
                                SignalTiming::CurrentClose};
  return BacktestRunner{asset,
                        market,
                        broker,
                        profile,
                        {},
                        BacktestRunner::PositionRule{},
                        std::move(short_position),
                        1'000.0};
}

TEST(PortfolioRunnerTest, UsesUnionTimelineAndMarksMissingBarsStale)
{
  const auto first = make_asset("First", {1.0, 2.0, 4.0});
  const auto second = make_asset("Second", {1.0, 3.0, 4.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile = Profile{"Profile"};
  auto runner = PortfolioRunner{
   1'000.0,
   10,
   10,
   {PortfolioRunner::BacktestRun{BacktestStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 make_runner(first, market, broker, profile)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_runner(second, market, broker, profile)}}};
  auto results = PortfolioResults{};

  while(results.timeline().size() < runner.total_timestamps()) {
    runner.run(results);
  }

  ASSERT_EQ(results.timeline().size(), 4);
  EXPECT_EQ(results.timeline().row(0).fresh_backtest_count, 2);
  EXPECT_EQ(results.timeline().row(1).fresh_backtest_count, 1);
  EXPECT_EQ(results.timeline().row(1).stale_backtest_count, 1);
  EXPECT_EQ(results.timeline().row(2).fresh_backtest_count, 1);
  EXPECT_EQ(results.timeline().row(3).fresh_backtest_count, 2);
  EXPECT_EQ(results.backtests()[0].timeline().size(), 3);
  EXPECT_EQ(results.backtests()[1].timeline().size(), 3);
}

TEST(PortfolioRunnerTest, RejectsNonIncreasingAssetTimestamps)
{
  const auto asset = make_asset("Invalid", {1.0, 1.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile = Profile{"Profile"};

  EXPECT_THROW(
   (PortfolioRunner{1'000.0,
                    10,
                    10,
                    {PortfolioRunner::BacktestRun{
                     BacktestStoreHandle{1, 1},
                     AssetStoreHandle{1, 1},
                     make_runner(asset, market, broker, profile)}}}),
   std::invalid_argument);
}

TEST(PortfolioRunnerTest, BacktestOrderHasDeterministicSharedCashPriority)
{
  const auto first = make_asset("First", {1.0});
  const auto second = make_asset("Second", {1.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   10,
   10,
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      second, market, broker, profile, InsufficientCashPolicy::Reject)}}};
  auto results = PortfolioResults{};

  runner.run(results);

  ASSERT_TRUE(results.backtests()[0].timeline().open_position(0));
  EXPECT_DOUBLE_EQ(
   results.backtests()[0].timeline().open_position(0)->position_size(), 8.0);
  EXPECT_FALSE(results.backtests()[1].timeline().open_position(0));
  ASSERT_EQ(results.backtests()[1].timeline().trade_events(0).size(), 1);
  EXPECT_TRUE(
   results.backtests()[1].timeline().trade_events(0).front().is_rejected());
  const auto& second_decision =
   results.backtests()[1].timeline().position_sizing_decisions(0).front();
  EXPECT_DOUBLE_EQ(*second_decision.requested_quantity, 8.0);
  EXPECT_DOUBLE_EQ(*second_decision.cash_available, 200.0);
  EXPECT_DOUBLE_EQ(results.timeline().row(0).reserved_notional, 800.0);
  EXPECT_DOUBLE_EQ(results.timeline().row(0).available_capital, 200.0);
}

TEST(PortfolioRunnerTest, HigherFirstComparatorOverridesBacktestOrder)
{
  const auto first = make_priced_asset("First", 90.0);
  const auto second = make_priced_asset("Second", 110.0);
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{9.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   1,
   10,
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      second, market, broker, profile, InsufficientCashPolicy::Reject)}},
   {PortfolioEntryComparator{RequestedOrderPriceNode{},
                             PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(0));
  EXPECT_TRUE(results.backtests()[1].timeline().open_position(0));
}

TEST(PortfolioRunnerTest, LowerRequestedNotionalExecutesFirst)
{
  const auto first = make_priced_asset("Larger", 100.0);
  const auto second = make_priced_asset("Smaller", 100.0);
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto larger =
   Profile{"Larger", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  const auto smaller =
   Profile{"Smaller", PositionSizingNode{FixedQuantityPositionSizing{2.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   1,
   10,
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, larger, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      second, market, broker, smaller, InsufficientCashPolicy::Reject)}},
   {PortfolioEntryComparator{RequestedNotionalNode{},
                             PortfolioEntryComparatorOrder::LowerFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(0));
  ASSERT_TRUE(results.backtests()[1].timeline().open_position(0));
  EXPECT_DOUBLE_EQ(
   results.backtests()[1].timeline().open_position(0)->position_size(), 2.0);
}

TEST(PortfolioRunnerTest, RejectedRankedOrderDoesNotBlockLaterOrder)
{
  const auto first = make_priced_asset("Unaffordable", 110.0);
  const auto second = make_priced_asset("Affordable", 100.0);
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto unaffordable = Profile{
   "Unaffordable", PositionSizingNode{FixedQuantityPositionSizing{20.0}}};
  const auto affordable =
   Profile{"Affordable", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   10,
   10,
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, unaffordable, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      second, market, broker, affordable, InsufficientCashPolicy::Reject)}},
   {PortfolioEntryComparator{RequestedOrderPriceNode{},
                             PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(0));
  EXPECT_TRUE(results.backtests()[1].timeline().open_position(0));
  ASSERT_EQ(
   results.backtests()[0].timeline().position_sizing_decisions(0).size(), 1U);
  EXPECT_EQ(results.backtests()[0]
             .timeline()
             .position_sizing_decisions(0)
             .front()
             .outcome,
            PositionSizingDecisionOutcome::InsufficientCash);
}

TEST(PortfolioRunnerTest, ComparatorsFallThroughBeforePortfolioOrder)
{
  const auto first = make_priced_asset("First", 90.0);
  const auto second = make_priced_asset("Second", 110.0);
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{9.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   1,
   10,
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      second, market, broker, profile, InsufficientCashPolicy::Reject)}},
   {PortfolioEntryComparator{ValueNode{1.0},
                             PortfolioEntryComparatorOrder::HigherFirst},
    PortfolioEntryComparator{RequestedOrderPriceNode{},
                             PortfolioEntryComparatorOrder::LowerFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_TRUE(results.backtests()[0].timeline().open_position(0));
  EXPECT_FALSE(results.backtests()[1].timeline().open_position(0));
}

TEST(PortfolioRunnerTest, FiniteComparatorScorePrecedesNonFiniteScore)
{
  const auto first = make_priced_asset("Non-finite", 100.0);
  const auto second = make_priced_asset("Finite", 110.0);
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   1,
   10,
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      second, market, broker, profile, InsufficientCashPolicy::Reject)}},
   {PortfolioEntryComparator{
    DivideNode{ValueNode{1.0},
               SubtractNode{RequestedOrderPriceNode{}, ValueNode{100.0}}},
    PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(0));
  EXPECT_TRUE(results.backtests()[1].timeline().open_position(0));
}

TEST(PortfolioRunnerTest, RequestedOrderProvidesEntryDirection)
{
  const auto short_asset = make_priced_asset("Short", 100.0);
  const auto long_asset = make_priced_asset("Long", 100.0);
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   1,
   10,
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_short_entry_runner(short_asset, market, broker, profile)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      long_asset, market, broker, profile, InsufficientCashPolicy::Reject)}},
   {PortfolioEntryComparator{RequestedOrderDirectionNode{},
                             PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(0));
  ASSERT_TRUE(results.backtests()[1].timeline().open_position(0));
  EXPECT_GT(results.backtests()[1].timeline().open_position(0)->position_size(),
            0.0);
}

TEST(PortfolioRunnerTest, ComparatorRejectsNamedStrategySeries)
{
  EXPECT_THROW(
   (PortfolioEntryComparator{SeriesNode{"priority"},
                             PortfolioEntryComparatorOrder::HigherFirst}),
   std::invalid_argument);
}

TEST(PortfolioRunnerTest, ComparatorRanksNextOpenOrdersAtOpenPhase)
{
  const auto first = Asset{"First",
                           AssetHistory{{"Datetime", {1.0, 2.0}},
                                        {"Open", {90.0, 90.0}},
                                        {"High", {90.0, 90.0}},
                                        {"Low", {90.0, 90.0}},
                                        {"Close", {90.0, 90.0}},
                                        {"Volume", {0.0, 0.0}}}};
  const auto second = Asset{"Second",
                            AssetHistory{{"Datetime", {1.0, 2.0}},
                                         {"Open", {110.0, 110.0}},
                                         {"High", {110.0, 110.0}},
                                         {"Low", {110.0, 110.0}},
                                         {"Close", {110.0, 110.0}},
                                         {"Volume", {0.0, 0.0}}}};
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  const auto make_next_open_runner = [&](const Asset& asset) {
    return BacktestRunner{asset,
                          market,
                          broker,
                          profile,
                          {},
                          BacktestRunner::PositionRule{BooleanMethod<true>{},
                                                       {},
                                                       BooleanMethod<false>{},
                                                       0,
                                                       0,
                                                       ValueMethod{10.0},
                                                       {},
                                                       SignalTiming::NextOpen},
                          BacktestRunner::PositionRule{},
                          1'000.0};
  };
  auto runner = PortfolioRunner{
   1'000.0,
   1,
   10,
   {PortfolioRunner::BacktestRun{BacktestStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 make_next_open_runner(first)},
    PortfolioRunner::BacktestRun{BacktestStoreHandle{2, 1},
                                 AssetStoreHandle{2, 1},
                                 make_next_open_runner(second)}},
   {PortfolioEntryComparator{RequestedOrderPriceNode{},
                             PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);
  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(1));
  EXPECT_TRUE(results.backtests()[1].timeline().open_position(1));
}

TEST(PortfolioRunnerTest, CapPolicyUsesOnlySharedAvailableCash)
{
  const auto first = make_asset("First", {1.0});
  const auto second = make_asset("Second", {1.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   10,
   10,
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(first,
                       market,
                       broker,
                       profile,
                       InsufficientCashPolicy::CapToAvailableCash)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(second,
                       market,
                       broker,
                       profile,
                       InsufficientCashPolicy::CapToAvailableCash)}}};
  auto results = PortfolioResults{};

  runner.run(results);

  ASSERT_TRUE(results.backtests()[1].timeline().open_position(0));
  EXPECT_DOUBLE_EQ(
   results.backtests()[1].timeline().open_position(0)->position_size(), 2.0);
  EXPECT_DOUBLE_EQ(results.timeline().row(0).reserved_notional, 1'000.0);
  EXPECT_DOUBLE_EQ(results.timeline().row(0).available_capital, 0.0);
}

TEST(PortfolioRunnerTest, ZeroLimitRejectsInitialEntryBeforeCashAdmission)
{
  const auto asset = make_asset("Blocked", {1.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{20.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   0,
   10,
   {PortfolioRunner::BacktestRun{
    BacktestStoreHandle{1, 1},
    AssetStoreHandle{1, 1},
    make_entry_runner(
     asset, market, broker, profile, InsufficientCashPolicy::Reject)}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(0));
  ASSERT_EQ(results.backtests()[0].timeline().trade_events(0).size(), 1);
  const auto& event = results.backtests()[0].timeline().trade_events(0).front();
  EXPECT_EQ(event.type(), TradeEvent::Type::rejected_maximum_open_trades);
  EXPECT_TRUE(event.is_rejected());
  EXPECT_DOUBLE_EQ(event.position_size(), 20.0);
  const auto& decision =
   results.backtests()[0].timeline().position_sizing_decisions(0).front();
  EXPECT_EQ(decision.outcome, PositionSizingDecisionOutcome::MaximumOpenTrades);
  EXPECT_FALSE(decision.cash_required);
  EXPECT_FALSE(decision.cash_available);
  EXPECT_DOUBLE_EQ(results.timeline().row(0).capital, 1'000.0);
  EXPECT_DOUBLE_EQ(results.timeline().row(0).reserved_notional, 0.0);
  EXPECT_EQ(results.timeline().row(0).open_position_count, 0);
}

TEST(PortfolioRunnerTest, OrderedEntriesStopAtMaximumOpenTrades)
{
  const auto first = make_asset("First", {1.0});
  const auto second = make_asset("Second", {1.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   1,
   10,
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      second, market, broker, profile, InsufficientCashPolicy::Reject)}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_TRUE(results.backtests()[0].timeline().open_position(0));
  EXPECT_FALSE(results.backtests()[1].timeline().open_position(0));
  ASSERT_EQ(results.backtests()[1].timeline().trade_events(0).size(), 1);
  EXPECT_EQ(results.backtests()[1].timeline().trade_events(0).front().type(),
            TradeEvent::Type::rejected_maximum_open_trades);
  EXPECT_EQ(results.timeline().row(0).open_position_count, 1);
}

TEST(PortfolioRunnerTest, FilteredEntryDoesNotConsumeOpenTradeSlot)
{
  const auto first = make_asset("Filtered", {1.0});
  const auto second = make_asset("Accepted", {1.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   1,
   1,
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(first,
                       market,
                       broker,
                       profile,
                       InsufficientCashPolicy::Reject,
                       BooleanMethod<false>{})},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      second, market, broker, profile, InsufficientCashPolicy::Reject)}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(0));
  EXPECT_TRUE(results.backtests()[1].timeline().open_position(0));
  EXPECT_TRUE(results.backtests()[0].timeline().trade_events(0).empty());
  EXPECT_EQ(results.timeline().row(0).open_position_count, 1);
}

TEST(PortfolioRunnerTest, ClosingTradeReleasesSlotAtSameTimestamp)
{
  const auto exiting = make_asset("Exiting", {1.0, 2.0});
  const auto entering = make_asset("Entering", {2.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto signal_exits =
   std::vector<BacktestRunner::PositionRule::SignalExitRule>{};
  signal_exits.emplace_back(
   true, BooleanMethod<true>{}, SignalTiming::NextOpen, 1.0);
  auto exiting_runner =
   BacktestRunner{exiting,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               std::move(signal_exits),
                                               BooleanMethod<false>{},
                                               1,
                                               0,
                                               ValueMethod{10.0},
                                               {}},
                  BacktestRunner::PositionRule{},
                  1'000.0};
  auto runner = PortfolioRunner{
   1'000.0,
   1,
   10,
   {PortfolioRunner::BacktestRun{BacktestStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 std::move(exiting_runner)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      entering, market, broker, profile, InsufficientCashPolicy::Reject)}}};
  auto results = PortfolioResults{};

  runner.run(results);
  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(1));
  EXPECT_TRUE(results.backtests()[1].timeline().open_position(0));
  EXPECT_EQ(results.timeline().row(1).open_position_count, 1);
}

TEST(PortfolioRunnerTest, PyramidingRemainsAllowedAtOpenTradeLimit)
{
  const auto asset = make_asset("Pyramid", {1.0, 2.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto pyramiding_runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               {},
                                               BooleanMethod<true>{},
                                               2,
                                               0,
                                               ValueMethod{10.0},
                                               {},
                                               SignalTiming::CurrentClose,
                                               SignalTiming::CurrentClose},
                  BacktestRunner::PositionRule{},
                  1'000.0};
  auto runner = PortfolioRunner{
   1'000.0,
   1,
   10,
   {PortfolioRunner::BacktestRun{BacktestStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 std::move(pyramiding_runner)}}};
  auto results = PortfolioResults{};

  runner.run(results);
  runner.run(results);

  ASSERT_TRUE(results.backtests()[0].timeline().open_position(1));
  EXPECT_DOUBLE_EQ(
   results.backtests()[0].timeline().open_position(1)->position_size(), 2.0);
  ASSERT_EQ(results.backtests()[0].timeline().trade_events(1).size(), 1);
  EXPECT_EQ(results.backtests()[0].timeline().trade_events(1).front().type(),
            TradeEvent::Type::scale_in);
}

TEST(PortfolioRunnerTest, ComparatorRanksPyramidingOrders)
{
  const auto first = Asset{"First",
                           AssetHistory{{"Datetime", {1.0, 2.0}},
                                        {"Open", {90.0, 90.0}},
                                        {"High", {90.0, 90.0}},
                                        {"Low", {90.0, 90.0}},
                                        {"Close", {90.0, 90.0}},
                                        {"Volume", {0.0, 0.0}}}};
  const auto second = Asset{"Second",
                            AssetHistory{{"Datetime", {1.0, 2.0}},
                                         {"Open", {110.0, 110.0}},
                                         {"High", {110.0, 110.0}},
                                         {"Low", {110.0, 110.0}},
                                         {"Close", {110.0, 110.0}},
                                         {"Volume", {0.0, 0.0}}}};
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  const auto make_pyramiding_runner = [&](const Asset& asset) {
    return BacktestRunner{
     asset,
     market,
     broker,
     profile,
     {},
     BacktestRunner::PositionRule{BooleanMethod<true>{},
                                  {},
                                  BooleanMethod<true>{},
                                  2,
                                  0,
                                  ValueMethod{10.0},
                                  {},
                                  SignalTiming::CurrentClose,
                                  SignalTiming::CurrentClose},
     BacktestRunner::PositionRule{},
     1'000.0};
  };
  auto runner = PortfolioRunner{
   1'000.0,
   10,
   3,
   {PortfolioRunner::BacktestRun{BacktestStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 make_pyramiding_runner(first)},
    PortfolioRunner::BacktestRun{BacktestStoreHandle{2, 1},
                                 AssetStoreHandle{2, 1},
                                 make_pyramiding_runner(second)}},
   {PortfolioEntryComparator{RequestedOrderPriceNode{},
                             PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);
  runner.run(results);

  ASSERT_TRUE(results.backtests()[0].timeline().open_position(1));
  ASSERT_TRUE(results.backtests()[1].timeline().open_position(1));
  EXPECT_DOUBLE_EQ(
   results.backtests()[0].timeline().open_position(1)->position_size(), 1.0);
  EXPECT_DOUBLE_EQ(
   results.backtests()[1].timeline().open_position(1)->position_size(), 2.0);
}

TEST(PortfolioRunnerTest, CombinedLayerLimitUsesDeterministicBacktestPriority)
{
  const auto first = make_asset("First", {1.0});
  const auto second = make_asset("Second", {1.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   10,
   1,
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_short_entry_runner(second, market, broker, profile)}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_TRUE(results.backtests()[0].timeline().open_position(0));
  EXPECT_FALSE(results.backtests()[1].timeline().open_position(0));
  EXPECT_TRUE(results.backtests()[1].timeline().strategy_intents(0).empty());
  ASSERT_EQ(results.backtests()[1].timeline().trade_events(0).size(), 1U);
  EXPECT_EQ(results.backtests()[1].timeline().trade_events(0).front().type(),
            TradeEvent::Type::rejected_maximum_combined_layers);
  ASSERT_EQ(
   results.backtests()[1].timeline().position_sizing_decisions(0).size(), 1U);
  const auto& decision =
   results.backtests()[1].timeline().position_sizing_decisions(0).front();
  EXPECT_EQ(decision.outcome,
            PositionSizingDecisionOutcome::MaximumCombinedLayers);
  EXPECT_EQ(decision.intent_id, 0U);
  EXPECT_EQ(decision.strategy_trade_id, 0U);
}

TEST(PortfolioRunnerTest, CombinedLayerLimitBlocksPyramidWithoutAdvancingState)
{
  const auto asset = Asset{"Pyramid",
                           AssetHistory{{"Datetime", {1.0, 2.0}},
                                        {"Open", {100.0, 110.0}},
                                        {"High", {100.0, 110.0}},
                                        {"Low", {100.0, 110.0}},
                                        {"Close", {100.0, 110.0}},
                                        {"Volume", {0.0, 0.0}}}};
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto stop_losses = std::vector<BacktestRunner::PositionRule::StopLossRule>{};
  stop_losses.emplace_back(
   SubtractMethod{StopTargetRefPriceMethod{}, ValueMethod{50.0}}, true, false);
  auto pyramiding_runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
                                {},
                                BooleanMethod<true>{},
                                3,
                                0,
                                ValueMethod{10.0},
                                std::move(stop_losses),
                                SignalTiming::CurrentClose,
                                SignalTiming::CurrentClose,
                                StopTargetReferencePrice::LatestEntryPrice,
                                StopTargetReferencePrice::LatestEntryPrice},
   BacktestRunner::PositionRule{},
   1'000.0};
  auto runner = PortfolioRunner{
   1'000.0,
   10,
   1,
   {PortfolioRunner::BacktestRun{BacktestStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 std::move(pyramiding_runner)}}};
  auto results = PortfolioResults{};

  runner.run(results);
  runner.run(results);

  const auto& timeline = results.backtests()[0].timeline();
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 1.0);
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->risk_reference_price(), 100.0);
  ASSERT_EQ(timeline.open_position(1)->stop_loss_levels().size(), 1U);
  EXPECT_DOUBLE_EQ(
   timeline.open_position(1)->stop_loss_levels().front().evaluated_price(),
   50.0);
  EXPECT_DOUBLE_EQ(
   timeline.open_position(1)->stop_loss_levels().front().effective_price(),
   50.0);
  EXPECT_TRUE(timeline.strategy_intents(1).empty());
  ASSERT_EQ(timeline.trade_events(1).size(), 1U);
  EXPECT_EQ(timeline.trade_events(1).front().type(),
            TradeEvent::Type::rejected_maximum_combined_layers);
  ASSERT_EQ(timeline.position_sizing_decisions(1).size(), 1U);
  EXPECT_EQ(timeline.position_sizing_decisions(1).front().outcome,
            PositionSizingDecisionOutcome::MaximumCombinedLayers);
}

TEST(PortfolioRunnerTest, ZeroCombinedLayerLimitRejectsInitialEntry)
{
  const auto asset = make_asset("Blocked", {1.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto runner = PortfolioRunner{
   1'000.0,
   10,
   0,
   {PortfolioRunner::BacktestRun{
    BacktestStoreHandle{1, 1},
    AssetStoreHandle{1, 1},
    make_entry_runner(
     asset, market, broker, profile, InsufficientCashPolicy::Reject)}}};
  auto results = PortfolioResults{};

  runner.run(results);

  const auto& timeline = results.backtests()[0].timeline();
  EXPECT_FALSE(timeline.open_position(0));
  EXPECT_TRUE(timeline.strategy_intents(0).empty());
  ASSERT_EQ(timeline.trade_events(0).size(), 1U);
  EXPECT_EQ(timeline.trade_events(0).front().type(),
            TradeEvent::Type::rejected_maximum_combined_layers);
}

TEST(PortfolioRunnerTest, FullClosureReleasesCombinedLayerAtSameTimestamp)
{
  const auto exiting = make_asset("Exiting", {1.0, 2.0});
  const auto entering = make_asset("Entering", {2.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto signal_exits =
   std::vector<BacktestRunner::PositionRule::SignalExitRule>{};
  signal_exits.emplace_back(
   true, BooleanMethod<true>{}, SignalTiming::NextOpen, 1.0);
  auto exiting_runner =
   BacktestRunner{exiting,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               std::move(signal_exits),
                                               BooleanMethod<false>{},
                                               1,
                                               0,
                                               ValueMethod{10.0},
                                               {}},
                  BacktestRunner::PositionRule{},
                  1'000.0};
  auto runner = PortfolioRunner{
   1'000.0,
   10,
   1,
   {PortfolioRunner::BacktestRun{BacktestStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 std::move(exiting_runner)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      entering, market, broker, profile, InsufficientCashPolicy::Reject)}}};
  auto results = PortfolioResults{};

  runner.run(results);
  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(1));
  EXPECT_TRUE(results.backtests()[1].timeline().open_position(0));
}
