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
   {PortfolioRunner::BacktestRun{BacktestStoreHandle{1, 1},
                                 make_runner(first, market, broker, profile)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1}, make_runner(second, market, broker, profile)}}};
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
   (PortfolioRunner{
    1'000.0,
    10,
    {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1}, make_runner(asset, market, broker, profile)}}}),
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
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
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
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     make_entry_runner(first,
                       market,
                       broker,
                       profile,
                       InsufficientCashPolicy::CapToAvailableCash)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
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
   {PortfolioRunner::BacktestRun{
    BacktestStoreHandle{1, 1},
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
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
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
   {PortfolioRunner::BacktestRun{
     BacktestStoreHandle{1, 1},
     make_entry_runner(first,
                       market,
                       broker,
                       profile,
                       InsufficientCashPolicy::Reject,
                       BooleanMethod<false>{})},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
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
   {PortfolioRunner::BacktestRun{BacktestStoreHandle{1, 1},
                                 std::move(exiting_runner)},
    PortfolioRunner::BacktestRun{
     BacktestStoreHandle{2, 1},
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
  auto runner =
   PortfolioRunner{1'000.0,
                   1,
                   {PortfolioRunner::BacktestRun{
                    BacktestStoreHandle{1, 1}, std::move(pyramiding_runner)}}};
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
