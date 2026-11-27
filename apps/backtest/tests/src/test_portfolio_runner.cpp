#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <memory>
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
                       ErasedSeriesMethod<EntryFilterMethodContext>
                        entry_filter = BooleanMethod<true>{}) -> BacktestRunner
{
  auto configured_profile = profile;
  configured_profile.insufficient_cash_policy(cash_policy);
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
                        configured_profile,
                        {},
                        std::move(long_position),
                        BacktestRunner::PositionRule{},
                        1'000.0,
                        0,
                        false,
                        NAN,
                        IntrabarPath::CandleDirection,
                        {},
                        std::move(entry_filter)};
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

auto make_performance_history_runner(
 const Asset& asset,
 const Market& market,
 const Broker& broker,
 const Profile& profile,
 ErasedSeriesMethod<BacktestMethodContext> entry,
 ErasedSeriesMethod<BacktestMethodContext> exit,
 ErasedSeriesMethod<BacktestMethodContext> pyramid = BooleanMethod<false>{},
 std::size_t maximum_layers = 1) -> BacktestRunner
{
  auto signal_exits = std::vector<BacktestRunner::PositionRule::SignalExitRule>{
   {true, std::move(exit), SignalTiming::CurrentClose}};
  auto long_position = BacktestRunner::PositionRule{
   std::move(entry),
   std::move(signal_exits),
   std::move(pyramid),
   maximum_layers,
   0,
   ValueMethod{10.0},
   {},
   SignalTiming::CurrentClose,
   SignalTiming::CurrentClose};
  return BacktestRunner{asset,
                        market,
                        broker,
                        profile,
                        {},
                        std::move(long_position),
                        BacktestRunner::PositionRule{},
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
    {PortfolioRunner::BacktestRun{SystemStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 make_runner(first, market, broker, profile)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
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

TEST(PortfolioRunnerTest, ExternalAssetUsesVisibleCandleWithoutAddingTimestamps)
{
  const auto bbca = Asset{"BBCA",
                          {{"Datetime", {1, 2, 4, 5}},
                           {"Open", {90, 95, 100, 105}},
                           {"High", {100, 100, 110, 110}},
                           {"Low", {90, 95, 100, 105}},
                           {"Close", {95, 100, 108, 110}}}};
  const auto ihsg = Asset{
   "IHSG",
   {{"Time", {1, 3, 4, 5}},
    {"Last", {6800, 7600, 7500, 6900}},
    {"risk_points", {8, 9, 12, 40}},
    {"short_column", {1}}},
   AssetQuoteFieldResolver{"Time", "Open", "High", "Low", "Last", "Volume"}};
  auto external = std::make_shared<ExternalAssetData>();
  external->add_field("market", "CLOSE", ihsg);
  external->add_field("market", "risk_points", ihsg);
  external->add_field("market", "short_column", ihsg);
  EXPECT_TRUE(std::isnan(external->value("market", "short_column", 4)));

  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  for(const auto timing :
      {SignalTiming::CurrentClose, SignalTiming::NextOpen}) {
    SCOPED_TRACE(static_cast<int>(timing));
    auto conversion = NodeToErasedMethodContext{};
    const auto entry = node_to_erased_method<BacktestMethodContext>(
     GreaterThanNode<BacktestMethodContext>{SeriesNode{"market_close"},
                                            ValueNode{7000}},
     conversion);
    auto series = StrategyMethodRegistry{};
    series.set("market_close",
               StrategyMethod{ExternFieldNode{"market", "CLOSE"}});
    series.set("market_close_alias",
               StrategyMethod{ExternFieldNode{"market", "CLOSE"}});
    series.set(
     "previous_market_close",
     node_to_erased_method<BacktestMethodContext>(
      LookbackNode<BacktestMethodContext>{SeriesNode{"market_close_alias"}, 1},
      conversion));
    auto position = BacktestRunner::PositionRule{
     entry,
     {},
     BooleanMethod<false>{},
     0,
     0,
     StrategyMethod{ExternFieldNode{"market", "risk_points"}},
     {},
     timing};
    auto setups = std::vector<BacktestRunner::CompiledStrategy>{};
    setups.emplace_back(
     std::move(series), std::move(position), BacktestRunner::PositionRule{});
    auto runner = PortfolioRunner{
     1'000.0,
     10,
     10,
     {PortfolioRunner::BacktestRun{SystemStoreHandle{1, 1},
                                   AssetStoreHandle{1, 1},
                                   BacktestRunner{bbca,
                                                  market,
                                                  broker,
                                                  profile,
                                                  std::move(setups),
                                                  1'000.0,
                                                  false,
                                                  NAN,
                                                  external}}}};
    auto results = PortfolioResults{};
    ASSERT_EQ(runner.total_timestamps(), 4U);
    while(runner.run_batch(results, 4) != 0) {
    }

    ASSERT_EQ(results.backtests().size(), 1U);
    const auto& backtest = results.backtests().front();
    const auto& timeline = backtest.timeline();
    const auto fill = timing == SignalTiming::CurrentClose ? 2U : 3U;
    ASSERT_EQ(results.events().size(), 1U);
    EXPECT_EQ(results.events().front().bar_index, fill);
    const auto opened = timeline.open_position(fill);
    ASSERT_TRUE(opened);
    EXPECT_DOUBLE_EQ(opened->entry_price(),
                     timing == SignalTiming::CurrentClose ? 108 : 105);
    EXPECT_DOUBLE_EQ(opened->risk_distance(), 12);
    const auto values =
     backtest.series_results(0).results(std::string{"market_close"});
    ASSERT_TRUE(values);
    ASSERT_EQ(values->get().size(), 4U);
    EXPECT_DOUBLE_EQ(values->get()[0], 6800);
    EXPECT_TRUE(std::isnan(values->get()[1]));
    EXPECT_DOUBLE_EQ(values->get()[2], 7500);
    EXPECT_DOUBLE_EQ(values->get()[3], 6900);
    const auto previous =
     backtest.series_results(0).results(std::string{"previous_market_close"});
    ASSERT_TRUE(previous);
    ASSERT_EQ(previous->get().size(), 4U);
    EXPECT_TRUE(std::isnan(previous->get()[0]));
    EXPECT_DOUBLE_EQ(previous->get()[1], 6800);
    EXPECT_TRUE(std::isnan(previous->get()[2]));
    EXPECT_DOUBLE_EQ(previous->get()[3], 7500);
  }
}

void expect_same_number(double batch_value, double single_value)
{
  if(std::isnan(batch_value) || std::isnan(single_value)) {
    EXPECT_EQ(std::isnan(batch_value), std::isnan(single_value));
    return;
  }
  EXPECT_DOUBLE_EQ(batch_value, single_value);
}

TEST(PortfolioRunnerTest, BatchRunMatchesSingleTimestampRun)
{
  const auto first = make_asset("First", {1.0, 2.0, 4.0, 6.0});
  const auto second = make_asset("Second", {1.0, 3.0, 4.0, 5.0});
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  const auto make_portfolio_runner = [&] {
    return PortfolioRunner{
     1'000.0,
     10,
     10,
     {PortfolioRunner::BacktestRun{
       SystemStoreHandle{1, 1},
       AssetStoreHandle{1, 1},
       make_entry_runner(
        first, market, broker, profile, InsufficientCashPolicy::Reject)},
      PortfolioRunner::BacktestRun{
       SystemStoreHandle{2, 1},
       AssetStoreHandle{2, 1},
       make_entry_runner(
        second, market, broker, profile, InsufficientCashPolicy::Reject)}}};
  };
  auto single_runner = make_portfolio_runner();
  auto batch_runner = make_portfolio_runner();
  auto single_results = PortfolioResults{};
  auto batch_results = PortfolioResults{};

  while(single_results.timeline().size() < single_runner.total_timestamps()) {
    single_runner.run(single_results);
  }
  while(batch_runner.run_batch(batch_results, 3) != 0) {
  }

  ASSERT_EQ(batch_results.timeline().size(), batch_runner.total_timestamps());
  EXPECT_EQ(batch_results.timeline(), single_results.timeline());
  EXPECT_EQ(batch_results.events(), single_results.events());
  ASSERT_EQ(batch_results.events().size(), 2U);
  for(std::size_t index = 0; index < batch_results.events().size(); ++index) {
    EXPECT_EQ(batch_results.events()[index], (PortfolioEventRef{index, 0, 0}));
  }
  ASSERT_EQ(batch_results.backtests().size(),
            single_results.backtests().size());
  for(auto index = std::size_t{}; index < batch_results.backtests().size();
      ++index) {
    EXPECT_EQ(batch_results.backtests()[index].key(),
              single_results.backtests()[index].key());
    const auto& batch_timeline = batch_results.backtests()[index].timeline();
    const auto& single_timeline = single_results.backtests()[index].timeline();
    ASSERT_EQ(batch_timeline.size(), single_timeline.size());
    for(auto row = std::size_t{}; row < batch_timeline.size(); ++row) {
      EXPECT_EQ(batch_timeline.market_timestamp(row),
                single_timeline.market_timestamp(row));
      expect_same_number(batch_timeline.market_price(row),
                         single_timeline.market_price(row));
      EXPECT_EQ(batch_timeline.market_lookback(row),
                single_timeline.market_lookback(row));
      expect_same_number(batch_timeline.capital(row),
                         single_timeline.capital(row));
      expect_same_number(batch_timeline.equity(row),
                         single_timeline.equity(row));
      expect_same_number(batch_timeline.peak_equity(row),
                         single_timeline.peak_equity(row));
      expect_same_number(batch_timeline.drawdown(row),
                         single_timeline.drawdown(row));
      expect_same_number(batch_timeline.max_drawdown(row),
                         single_timeline.max_drawdown(row));
      ASSERT_EQ(batch_timeline.trade_events(row).size(),
                single_timeline.trade_events(row).size());
      for(auto event = std::size_t{};
          event < batch_timeline.trade_events(row).size();
          ++event) {
        const auto& batch_event = batch_timeline.trade_events(row)[event];
        const auto& single_event = single_timeline.trade_events(row)[event];
        EXPECT_EQ(batch_event.trade_id(), single_event.trade_id());
        EXPECT_EQ(batch_event.event_id(), single_event.event_id());
        EXPECT_EQ(batch_event.trade_event_index(),
                  single_event.trade_event_index());
        EXPECT_EQ(batch_event.type(), single_event.type());
        EXPECT_EQ(batch_event.timestamp(), single_event.timestamp());
        expect_same_number(batch_event.price(), single_event.price());
        expect_same_number(batch_event.position_size(),
                           single_event.position_size());
        expect_same_number(batch_event.fees(), single_event.fees());
        EXPECT_EQ(batch_event.strategy_index(), single_event.strategy_index());
      }
      EXPECT_EQ(batch_timeline.closed_trades(row).size(),
                single_timeline.closed_trades(row).size());
      ASSERT_EQ(batch_timeline.open_position(row).has_value(),
                single_timeline.open_position(row).has_value());
      if(batch_timeline.open_position(row)) {
        const auto& batch_position = *batch_timeline.open_position(row);
        const auto& single_position = *single_timeline.open_position(row);
        EXPECT_EQ(batch_position.trade_id(), single_position.trade_id());
        EXPECT_EQ(batch_position.strategy_index(),
                  single_position.strategy_index());
        EXPECT_EQ(batch_position.entry_timestamp(),
                  single_position.entry_timestamp());
        EXPECT_EQ(batch_position.market_timestamp(),
                  single_position.market_timestamp());
        expect_same_number(batch_position.market_price(),
                           single_position.market_price());
        expect_same_number(batch_position.position_size(),
                           single_position.position_size());
        expect_same_number(batch_position.investment(),
                           single_position.investment());
      }
    }
  }
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
                      SystemStoreHandle{1, 1},
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
      SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
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

TEST(PortfolioRunnerTest, CurrentCloseAssetComparatorOverridesBacktestOrder)
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
      SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      second, market, broker, profile, InsufficientCashPolicy::Reject)}},
   {PortfolioEntryComparator{CloseNode{},
                             PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(0));
  EXPECT_TRUE(results.backtests()[1].timeline().open_position(0));
}

TEST(PortfolioRunnerTest, EventsRecordRankedFillBeforeRejectedAlternateRequest)
{
  const auto first = make_priced_asset("First", 90.0);
  const auto second = make_priced_asset("Second", 110.0);
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{9.0}}};
  profile.insufficient_cash_policy(InsufficientCashPolicy::Reject);
  const auto entry = BacktestRunner::PositionRule{BooleanMethod<true>{},
                                                  {},
                                                  BooleanMethod<false>{},
                                                  0,
                                                  0,
                                                  ValueMethod{10.0},
                                                  {}};
  auto strategies = std::vector<BacktestRunner::CompiledStrategy>{
   {{}, {}, {}}, {{}, entry, {}}};
  auto runner = PortfolioRunner{
   1'000.0,
   10,
   10,
   {{SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     BacktestRunner{first, market, broker, profile, std::move(strategies), 1'000.0}},
    {SystemStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      second, market, broker, profile, InsufficientCashPolicy::Reject)}},
   {PortfolioEntryComparator{CloseNode{},
                             PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);

  ASSERT_EQ(results.events().size(), 2U);
  const auto& fill = results.events()[0];
  ASSERT_EQ(fill, (PortfolioEventRef{1, 0, 0}));
  const auto& entry_event = results.backtests()[fill.backtest_index]
                             .timeline()
                             .trade_events(fill.bar_index)
                             .at(fill.event_index);
  EXPECT_EQ(entry_event.type(), TradeEvent::Type::entry);
  EXPECT_EQ(entry_event.strategy_index(), 0U);

  const auto& rejection = results.events()[1];
  ASSERT_EQ(rejection, (PortfolioEventRef{0, 0, 0}));
  const auto& rejected = results.backtests()[rejection.backtest_index]
                          .timeline()
                          .trade_events(rejection.bar_index)
                          .at(rejection.event_index);
  EXPECT_EQ(rejected.timestamp(), entry_event.timestamp());
  EXPECT_EQ(rejected.type(), TradeEvent::Type::rejected_insufficient_cash);
  EXPECT_EQ(rejected.strategy_index(), 1U);
  EXPECT_EQ(rejected.trade_id(), 0U);
  EXPECT_DOUBLE_EQ(rejected.position_size(), 9.0);
  EXPECT_DOUBLE_EQ(rejected.price(), 90.0);
  EXPECT_DOUBLE_EQ(rejected.rejection_required_cash(), 810.0);
  EXPECT_DOUBLE_EQ(rejected.rejection_available_cash(), 10.0);
}

TEST(PortfolioRunnerTest, EventsIncludeEachIntrabarStopInExecutionOrder)
{
  const auto asset = Asset{"Short",
                           AssetHistory{{"Datetime", {1.0, 2.0}},
                                        {"Open", {100.0, 115.0}},
                                        {"High", {100.0, 125.0}},
                                        {"Low", {100.0, 100.0}},
                                        {"Close", {100.0, 115.0}},
                                        {"Volume", {0.0, 0.0}}}};
  const auto market = Market{"Market", 1.0, 1.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto short_entry = BacktestRunner::PositionRule{
   BooleanMethod<true>{},
   {},
   BooleanMethod<false>{},
   1,
   0,
   ValueMethod{10.0},
   {{ValueMethod{110.0}, true, false, 0.5},
    {ValueMethod{120.0}, true, false, 1.0}}};
  auto runner = PortfolioRunner{
   5'000.0,
   10,
   10,
   {{SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     BacktestRunner{
      asset, market, broker, profile, {}, {}, std::move(short_entry), 5'000.0}}}};
  auto results = PortfolioResults{};

  ASSERT_EQ(runner.run_batch(results, 2), 2U);

  const auto expected =
   std::vector<PortfolioEventRef>{{0, 0, 0}, {0, 1, 0}, {0, 1, 1}};
  ASSERT_EQ(results.events(), expected);
  const auto& stops = results.backtests()[0].timeline().trade_events(1);
  ASSERT_EQ(stops.size(), 2U);
  EXPECT_EQ(stops[0].type(), TradeEvent::Type::stop_loss);
  EXPECT_EQ(stops[1].type(), TradeEvent::Type::stop_loss);
  EXPECT_DOUBLE_EQ(stops[0].price(), 115.0);
  EXPECT_DOUBLE_EQ(stops[1].price(), 120.0);
  EXPECT_DOUBLE_EQ(stops[0].position_size_after(), -4.0);
  EXPECT_DOUBLE_EQ(stops[1].position_size_after(), 0.0);
}

TEST(PortfolioRunnerTest, IntrabarPathControlsActualAndHypotheticalExits)
{
  const auto asset = Asset{"Both levels touched",
                           AssetHistory{{"Datetime", {1.0, 2.0}},
                                        {"Open", {100.0, 100.0}},
                                        {"High", {100.0, 110.0}},
                                        {"Low", {100.0, 90.0}},
                                        {"Close", {100.0, 108.0}},
                                        {"Volume", {0.0, 0.0}}}};
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  const auto long_rule =
   BacktestRunner::PositionRule{BooleanMethod<true>{},
                                {},
                                BooleanMethod<false>{},
                                1,
                                0,
                                ValueMethod{5.0},
                                {{ValueMethod{95.0}, true}},
                                SignalTiming::CurrentClose,
                                SignalTiming::NextOpen,
                                StopTargetReferencePrice::AveragePrice,
                                StopTargetReferencePrice::AveragePrice,
                                {{ValueMethod{105.0}, true}}};

  struct Scenario {
    IntrabarPath path;
    double price;
    TradeEvent::Type actual_exit;
    StrategyIntentType hypothetical_exit;
  };

  const auto scenarios = std::array{Scenario{IntrabarPath::LowFirst,
                                             95.0,
                                             TradeEvent::Type::stop_loss,
                                             StrategyIntentType::StopLoss},
                                    Scenario{IntrabarPath::HighFirst,
                                             105.0,
                                             TradeEvent::Type::take_profit,
                                             StrategyIntentType::TakeProfit},
                                    Scenario{IntrabarPath::CandleDirection,
                                             95.0,
                                             TradeEvent::Type::stop_loss,
                                             StrategyIntentType::StopLoss}};

  for(const auto& scenario : scenarios) {
    SCOPED_TRACE(static_cast<int>(scenario.path));
    const auto standalone_path = scenario.path == IntrabarPath::HighFirst
                                  ? IntrabarPath::LowFirst
                                  : IntrabarPath::HighFirst;
    auto main = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               long_rule,
                               {},
                               1'000.0,
                               0,
                               false,
                               NAN,
                               standalone_path};
    auto setups = std::vector<BacktestRunner::CompiledStrategy>{
     {{}, long_rule, {}, {}, BooleanMethod<false>{}}, {{}, long_rule, {}}};
    auto alternate =
     BacktestRunner{asset, market, broker, profile, std::move(setups), 1'000.0};
    auto runner = PortfolioRunner{
     1'000.0,
     10,
     10,
     {{SystemStoreHandle{1, 1}, AssetStoreHandle{1, 1}, std::move(main)},
      {SystemStoreHandle{2, 1}, AssetStoreHandle{1, 1}, std::move(alternate)}},
     {},
     scenario.path};
    auto results = PortfolioResults{};

    ASSERT_EQ(runner.run_batch(results, 2), 2U);

    ASSERT_EQ(results.backtests().size(), 2U);
    for(auto index = std::size_t{}; index < results.backtests().size();
        ++index) {
      const auto& timeline = results.backtests()[index].timeline();
      const auto& actual = timeline.closed_trades(1);
      ASSERT_EQ(actual.size(), 1U);
      EXPECT_EQ(actual.front().strategy_index(), index);
      EXPECT_EQ(actual.front().exit_type(), scenario.actual_exit);
      EXPECT_DOUBLE_EQ(actual.front().exit_price(), scenario.price);

      ASSERT_EQ(timeline.strategy_count(1), index + 1);
      for(auto setup = std::size_t{}; setup < timeline.strategy_count(1);
          ++setup) {
        const auto& hypothetical =
         timeline.strategy_state(1, setup).strategy_closed_positions;
        ASSERT_EQ(hypothetical.size(), 1U);
        const auto& intents = hypothetical.front().intents();
        ASSERT_EQ(intents.size(), 2U);
        EXPECT_EQ(intents.back().type(), scenario.hypothetical_exit);
        EXPECT_DOUBLE_EQ(intents.back().price(), scenario.price);
      }
    }
  }
}

TEST(PortfolioRunnerTest, IntrabarPathControlsSameCandleTrailingStop)
{
  const auto asset = Asset{"Trailing stop",
                           AssetHistory{{"Datetime", {1.0, 2.0}},
                                        {"Open", {100.0, 100.0}},
                                        {"High", {100.0, 110.0}},
                                        {"Low", {100.0, 97.0}},
                                        {"Close", {100.0, 108.0}},
                                        {"Volume", {0.0, 0.0}}}};
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  const auto long_rule =
   BacktestRunner::PositionRule{BooleanMethod<true>{},
                                {},
                                BooleanMethod<false>{},
                                1,
                                0,
                                ValueMethod{5.0},
                                {{ValueMethod{95.0}, true, true}}};
  const auto run = [&](IntrabarPath path) {
    auto runner = PortfolioRunner{
     1'000.0,
     10,
     10,
     {{SystemStoreHandle{1, 1},
       AssetStoreHandle{1, 1},
       BacktestRunner{
        asset, market, broker, profile, {}, long_rule, {}, 1'000.0}}},
     {},
     path};
    auto results = PortfolioResults{};
    EXPECT_EQ(runner.run_batch(results, 2), 2U);
    return results;
  };

  const auto low_first = run(IntrabarPath::LowFirst);
  const auto& low_timeline = low_first.backtests().front().timeline();
  EXPECT_TRUE(low_timeline.closed_trades(1).empty());
  EXPECT_TRUE(low_timeline.open_position(1));

  const auto high_first = run(IntrabarPath::HighFirst);
  const auto& high_timeline = high_first.backtests().front().timeline();
  const auto& exits = high_timeline.closed_trades(1);
  ASSERT_EQ(exits.size(), 1U);
  EXPECT_EQ(exits.front().exit_type(), TradeEvent::Type::stop_loss);
  EXPECT_DOUBLE_EQ(exits.front().exit_price(), 105.0);
}

TEST(PortfolioRunnerTest,
     StrategyPerformanceComparatorUsesEachRequestingStrategySnapshot)
{
  const auto without_history = Asset{
   "Without history",
   AssetHistory{{"Datetime", {1.0, 2.0, 3.0}},
                {"Open", {90.0, 90.0, 120.0}},
                {"High", {90.0, 90.0, 120.0}},
                {"Low", {90.0, 90.0, 120.0}},
                {"Close", {90.0, 90.0, 120.0}},
                {"Volume", {0.0, 0.0, 0.0}}}};
  const auto with_history = Asset{
   "With history",
   AssetHistory{{"Datetime", {1.0, 2.0, 3.0}},
                {"Open", {100.0, 110.0, 120.0}},
                {"High", {100.0, 110.0, 120.0}},
                {"Low", {100.0, 110.0, 120.0}},
                {"Close", {100.0, 110.0, 120.0}},
                {"Volume", {0.0, 0.0, 0.0}}}};
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  const auto entry_at_120 =
   EqualMethod{CloseMethod{}, ValueMethod{120.0}};
  auto runner = PortfolioRunner{
   1'000.0,
   1,
   10,
   {PortfolioRunner::BacktestRun{
     SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_performance_history_runner(without_history,
                                     market,
                                     broker,
                                     profile,
                                     entry_at_120,
                                     BooleanMethod<false>{})},
    PortfolioRunner::BacktestRun{
     SystemStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_performance_history_runner(
      with_history,
      market,
      broker,
      profile,
      LogicalOrMethod{EqualMethod{CloseMethod{}, ValueMethod{100.0}},
                      entry_at_120},
      EqualMethod{CloseMethod{}, ValueMethod{110.0}})}},
   {PortfolioEntryComparator{
    StrategyPerformanceNode{StrategyPerformanceMetric::LifetimeCount},
    PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);
  runner.run(results);
  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(2));
  EXPECT_TRUE(results.backtests()[1].timeline().open_position(2));
  ASSERT_EQ(results.backtests()[0].timeline().trade_events(2).size(), 1U);
  EXPECT_TRUE(
   results.backtests()[0].timeline().trade_events(2).front().is_rejected());
}

TEST(PortfolioRunnerTest,
     StrategyPerformanceComparatorRanksPyramidsByOwnerSnapshot)
{
  const auto without_history = Asset{
   "Without history",
   AssetHistory{{"Datetime", {1.0, 2.0, 3.0, 4.0}},
                {"Open", {90.0, 90.0, 120.0, 130.0}},
                {"High", {90.0, 90.0, 120.0, 130.0}},
                {"Low", {90.0, 90.0, 120.0, 130.0}},
                {"Close", {90.0, 90.0, 120.0, 130.0}},
                {"Volume", {0.0, 0.0, 0.0, 0.0}}}};
  const auto with_history = Asset{
   "With history",
   AssetHistory{{"Datetime", {1.0, 2.0, 3.0, 4.0}},
                {"Open", {100.0, 110.0, 120.0, 130.0}},
                {"High", {100.0, 110.0, 120.0, 130.0}},
                {"Low", {100.0, 110.0, 120.0, 130.0}},
                {"Close", {100.0, 110.0, 120.0, 130.0}},
                {"Volume", {0.0, 0.0, 0.0, 0.0}}}};
  const auto market = Market{"Market", 0.0, 0.0};
  const auto broker = Broker{"Broker"};
  const auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  const auto entry_at_120 =
   EqualMethod{CloseMethod{}, ValueMethod{120.0}};
  const auto pyramid_at_130 =
   EqualMethod{CloseMethod{}, ValueMethod{130.0}};
  auto runner = PortfolioRunner{
   1'000.0,
   2,
   3,
   {PortfolioRunner::BacktestRun{
     SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_performance_history_runner(without_history,
                                     market,
                                     broker,
                                     profile,
                                     entry_at_120,
                                     BooleanMethod<false>{},
                                     pyramid_at_130,
                                     2)},
    PortfolioRunner::BacktestRun{
     SystemStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_performance_history_runner(
      with_history,
      market,
      broker,
      profile,
      LogicalOrMethod{EqualMethod{CloseMethod{}, ValueMethod{100.0}},
                      entry_at_120},
      EqualMethod{CloseMethod{}, ValueMethod{110.0}},
      pyramid_at_130,
      2)}},
   {PortfolioEntryComparator{
    StrategyPerformanceNode{StrategyPerformanceMetric::LifetimeCount},
    PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);
  runner.run(results);
  runner.run(results);
  runner.run(results);

  ASSERT_TRUE(results.backtests()[0].timeline().open_position(3));
  ASSERT_TRUE(results.backtests()[1].timeline().open_position(3));
  EXPECT_DOUBLE_EQ(
   results.backtests()[0].timeline().open_position(3)->position_size(), 1.0);
  EXPECT_DOUBLE_EQ(
   results.backtests()[1].timeline().open_position(3)->position_size(), 2.0);
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
      SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, larger, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
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
      SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, unaffordable, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
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
      SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
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
      SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      second, market, broker, profile, InsufficientCashPolicy::Reject)}},
   {PortfolioEntryComparator{
    DivideNode<RequestedOrderMethodContext>{
     ValueNode{1.0},
     SubtractNode<RequestedOrderMethodContext>{RequestedOrderPriceNode{},
                                               ValueNode{100.0}}},
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
      SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_short_entry_runner(short_asset, market, broker, profile)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
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

TEST(PortfolioRunnerTest, NextOpenComparatorUsesPreviousCompletedAssetBar)
{
  const auto first = Asset{"First",
                           AssetHistory{{"Datetime", {1.0, 2.0}},
                                        {"Open", {200.0, 100.0}},
                                        {"High", {200.0, 100.0}},
                                        {"Low", {200.0, 1.0}},
                                        {"Close", {200.0, 1.0}},
                                        {"Volume", {0.0, 0.0}}}};
  const auto second = Asset{"Second",
                            AssetHistory{{"Datetime", {1.0, 2.0}},
                                         {"Open", {100.0, 100.0}},
                                         {"High", {100.0, 1'000.0}},
                                         {"Low", {100.0, 100.0}},
                                         {"Close", {100.0, 1'000.0}},
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
    {PortfolioRunner::BacktestRun{SystemStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 make_next_open_runner(first)},
     PortfolioRunner::BacktestRun{SystemStoreHandle{2, 1},
                                 AssetStoreHandle{2, 1},
                                 make_next_open_runner(second)}},
   {PortfolioEntryComparator{CloseNode{},
                             PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};

  runner.run(results);
  runner.run(results);

  EXPECT_TRUE(results.backtests()[0].timeline().open_position(1));
  EXPECT_FALSE(results.backtests()[1].timeline().open_position(1));
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
      SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(first,
                       market,
                       broker,
                       profile,
                       InsufficientCashPolicy::CapToAvailableCash)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
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
     SystemStoreHandle{1, 1},
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
     SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
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

TEST(PortfolioRunnerTest, EntryFilteredOrderDoesNotConsumeOpenTradeSlot)
{
  const auto first = make_asset("Entry filtered", {1.0});
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
      SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(first,
                       market,
                       broker,
                       profile,
                       InsufficientCashPolicy::Reject,
                       BooleanMethod<false>{})},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
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
   {PortfolioRunner::BacktestRun{
     SystemStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      entering, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{SystemStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 std::move(exiting_runner)}}};
  auto results = PortfolioResults{};

  runner.run(results);
  runner.run(results);

  EXPECT_FALSE(results.backtests()[1].timeline().open_position(1));
  EXPECT_TRUE(results.backtests()[0].timeline().open_position(0));
  EXPECT_EQ(results.timeline().row(1).open_position_count, 1);
  ASSERT_GE(results.events().size(), 3U);
  const auto& exit = results.events()[1];
  ASSERT_EQ(exit, (PortfolioEventRef{1, 1, 0}));
  const auto& exit_event = results.backtests()[exit.backtest_index]
                            .timeline()
                            .trade_events(exit.bar_index)
                            .at(exit.event_index);
  const auto& entry = results.events()[2];
  ASSERT_EQ(entry, (PortfolioEventRef{0, 0, 0}));
  const auto& entry_event = results.backtests()[entry.backtest_index]
                             .timeline()
                             .trade_events(entry.bar_index)
                             .at(entry.event_index);
  EXPECT_EQ(exit_event.type(), TradeEvent::Type::exit_signal);
  EXPECT_EQ(entry_event.type(), TradeEvent::Type::entry);
  EXPECT_EQ(exit_event.timestamp(), entry_event.timestamp());
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
    {PortfolioRunner::BacktestRun{SystemStoreHandle{1, 1},
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
    {PortfolioRunner::BacktestRun{SystemStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 make_pyramiding_runner(first)},
     PortfolioRunner::BacktestRun{SystemStoreHandle{2, 1},
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
      SystemStoreHandle{1, 1},
     AssetStoreHandle{1, 1},
     make_entry_runner(
      first, market, broker, profile, InsufficientCashPolicy::Reject)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_short_entry_runner(second, market, broker, profile)}}};
  auto results = PortfolioResults{};

  runner.run(results);

  EXPECT_TRUE(results.backtests()[0].timeline().open_position(0));
  EXPECT_FALSE(results.backtests()[1].timeline().open_position(0));
  EXPECT_TRUE(results.backtests()[1]
               .timeline()
                .strategy_state(0, 0)
                .strategy_intents.empty());
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
    {PortfolioRunner::BacktestRun{SystemStoreHandle{1, 1},
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
   EXPECT_TRUE(timeline.strategy_state(1, 0).strategy_intents.empty());
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
     SystemStoreHandle{1, 1},
    AssetStoreHandle{1, 1},
    make_entry_runner(
     asset, market, broker, profile, InsufficientCashPolicy::Reject)}}};
  auto results = PortfolioResults{};

  runner.run(results);

  const auto& timeline = results.backtests()[0].timeline();
  EXPECT_FALSE(timeline.open_position(0));
   EXPECT_TRUE(timeline.strategy_state(0, 0).strategy_intents.empty());
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
    {PortfolioRunner::BacktestRun{SystemStoreHandle{1, 1},
                                 AssetStoreHandle{1, 1},
                                 std::move(exiting_runner)},
    PortfolioRunner::BacktestRun{
      SystemStoreHandle{2, 1},
     AssetStoreHandle{2, 1},
     make_entry_runner(
      entering, market, broker, profile, InsufficientCashPolicy::Reject)}}};
  auto results = PortfolioResults{};

  runner.run(results);
  runner.run(results);

  EXPECT_FALSE(results.backtests()[0].timeline().open_position(1));
  EXPECT_TRUE(results.backtests()[1].timeline().open_position(0));
}
