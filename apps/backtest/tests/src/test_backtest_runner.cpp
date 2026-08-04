#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

auto make_single_bar_asset(double open_price = 100.0) -> Asset
{
  return Asset{"Test",
               AssetHistory{{"Datetime", {1.0}},
                            {"Open", {open_price}},
                            {"High", {open_price}},
                            {"Low", {open_price}},
                            {"Close", {open_price}},
                            {"Volume", {0.0}}}};
}

auto make_single_bar_asset_with_close(double open_price, double close_price)
 -> Asset
{
  return Asset{"Test",
               AssetHistory{{"Datetime", {1.0}},
                            {"Open", {open_price}},
                            {"High", {std::max(open_price, close_price)}},
                            {"Low", {std::min(open_price, close_price)}},
                            {"Close", {close_price}},
                            {"Volume", {0.0}}}};
}

auto make_single_bar_asset_with_range(double open_price,
                                      double high_price,
                                      double low_price,
                                      double close_price) -> Asset
{
  return Asset{"Test",
               AssetHistory{{"Datetime", {1.0}},
                            {"Open", {open_price}},
                            {"High", {high_price}},
                            {"Low", {low_price}},
                            {"Close", {close_price}},
                            {"Volume", {0.0}}}};
}

auto make_two_bar_asset(double first_open,
                        double first_high,
                        double first_low,
                        double first_close,
                        double second_open,
                        double second_high,
                        double second_low,
                        double second_close) -> Asset;

TEST(IntrabarPathTest, UsesConfiguredAndCandleDirectionPaths)
{
  EXPECT_EQ(
   make_intrabar_prices(IntrabarPath::LowFirst, 100.0, 120.0, 80.0, 110.0),
   (IntrabarPrices{100.0, 80.0, 120.0, 110.0}));
  EXPECT_EQ(
   make_intrabar_prices(IntrabarPath::HighFirst, 100.0, 120.0, 80.0, 90.0),
   (IntrabarPrices{100.0, 120.0, 80.0, 90.0}));
  EXPECT_EQ(make_intrabar_prices(
             IntrabarPath::CandleDirection, 100.0, 120.0, 80.0, 110.0),
            (IntrabarPrices{100.0, 80.0, 120.0, 110.0}));
  EXPECT_EQ(make_intrabar_prices(
             IntrabarPath::CandleDirection, 100.0, 120.0, 80.0, 90.0),
            (IntrabarPrices{100.0, 120.0, 80.0, 90.0}));
}

TEST(IntrabarPathTest, DojiVisitsNearestExtremeAndBreaksTieLowFirst)
{
  EXPECT_EQ(make_intrabar_prices(
             IntrabarPath::CandleDirection, 100.0, 130.0, 90.0, 100.0),
            (IntrabarPrices{100.0, 90.0, 130.0, 100.0}));
  EXPECT_EQ(make_intrabar_prices(
             IntrabarPath::CandleDirection, 100.0, 120.0, 80.0, 100.0),
            (IntrabarPrices{100.0, 80.0, 120.0, 100.0}));
}

auto make_position_rule(
 ErasedSeriesMethod<ErasedSeriesMethodContext> entry_method,
 ErasedSeriesMethod<ErasedSeriesMethodContext> exit_method,
 ErasedSeriesMethod<ErasedSeriesMethodContext> pyramiding_signal,
 std::size_t pyramiding_max_layers,
 ErasedSeriesMethod<ErasedSeriesMethodContext> stop_price_method,
 bool stop_loss_enabled,
 bool stop_loss_trailing_enabled,
 std::size_t entry_signal_delay = 1,
 ErasedSeriesMethod<ErasedSeriesMethodContext> entry_price_method =
  OpenMethod{},
 std::size_t exit_signal_delay = 1,
 ErasedSeriesMethod<ErasedSeriesMethodContext> exit_price_method = OpenMethod{},
 std::size_t pyramiding_signal_delay = 1,
 ErasedSeriesMethod<ErasedSeriesMethodContext> pyramiding_price_method =
  OpenMethod{},
 StopTargetReferencePrice favorable_stop_target_reference =
  StopTargetReferencePrice::AveragePrice,
 StopTargetReferencePrice unfavorable_stop_target_reference =
  StopTargetReferencePrice::AveragePrice,
 double signal_exit_reduce = 1.0,
 double stop_loss_reduce = 1.0,
 std::vector<BacktestRunner::PositionRule::TakeProfitRule> take_profits = {},
 ErasedSeriesMethod<ErasedSeriesMethodContext> risk_distance_method =
  ValueMethod{10.0}) -> BacktestRunner::PositionRule;

auto run_single_entry(
 PositionSizingNode position_sizing,
 double entry_price = 100.0,
 DrawdownAdjustment drawdown_adjustment = {},
 double initial_capital = 1000.0,
 double peak_equity = std::numeric_limits<double>::quiet_NaN())
 -> BacktestTimeline
{
  const auto asset = make_single_bar_asset(entry_price);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile = Profile{"Test", position_sizing, drawdown_adjustment};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<false>{},
                      1,
                      ValueMethod{std::numeric_limits<double>::quiet_NaN()},
                      false,
                      false),
   BacktestRunner::PositionRule{},
   initial_capital,
   0,
   false,
   peak_equity};

  runner.run(series_results, timeline);

  return timeline;
}

auto run_single_close_price_entry(
 PositionSizingNode position_sizing,
 double open_price = 100.0,
 double close_price = 125.0,
 double stop_price = std::numeric_limits<double>::quiet_NaN(),
 Broker broker = Broker{"Test"},
 double risk_distance = 10.0) -> BacktestTimeline
{
  const auto asset = make_single_bar_asset_with_close(open_price, close_price);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto profile = Profile{"Test", position_sizing};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  make_position_rule(BooleanMethod<true>{},
                                     BooleanMethod<false>{},
                                     BooleanMethod<false>{},
                                     1,
                                     ValueMethod{stop_price},
                                     false,
                                     false,
                                     0,
                                     CloseMethod{},
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     1.0,
                                     1.0,
                                     {},
                                     ValueMethod{risk_distance}),
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);

  return timeline;
}

auto last_timeline_index(const BacktestTimeline& timeline) -> std::size_t
{
  return timeline.size() - 1;
}

auto latest_position(const BacktestTimeline& timeline)
 -> const OpenPositionSnapshot&
{
  const auto timeline_i = last_timeline_index(timeline);
  return *timeline.open_position(timeline_i);
}

auto stop_level(const auto& state, std::size_t index = 0)
 -> const StopLossLevel&
{
  return state.stop_loss_levels().at(index);
}

auto latest_closed_trade(const BacktestTimeline& timeline) -> const ClosedTrade&
{
  const auto timeline_i = last_timeline_index(timeline);
  return timeline.closed_trades(timeline_i).back();
}

auto single_take_profit(
 ErasedSeriesMethod<ErasedSeriesMethodContext> target_price,
 bool enabled,
 double reduce = 1.0)
 -> std::vector<BacktestRunner::PositionRule::TakeProfitRule>
{
  auto take_profits =
   std::vector<BacktestRunner::PositionRule::TakeProfitRule>{};
  take_profits.emplace_back(std::move(target_price), enabled, reduce);
  return take_profits;
}

auto single_stop_loss(ErasedSeriesMethod<ErasedSeriesMethodContext> stop_price,
                      bool enabled,
                      bool trailing = false,
                      double reduce = 1.0)
 -> std::vector<BacktestRunner::PositionRule::StopLossRule>
{
  auto stop_losses = std::vector<BacktestRunner::PositionRule::StopLossRule>{};
  stop_losses.emplace_back(std::move(stop_price), enabled, trailing, reduce);
  return stop_losses;
}

auto make_position_rule(
 ErasedSeriesMethod<ErasedSeriesMethodContext> entry_method,
 ErasedSeriesMethod<ErasedSeriesMethodContext> exit_method,
 ErasedSeriesMethod<ErasedSeriesMethodContext> pyramiding_signal,
 std::size_t pyramiding_max_layers,
 ErasedSeriesMethod<ErasedSeriesMethodContext> stop_price_method,
 bool stop_loss_enabled,
 bool stop_loss_trailing_enabled,
 std::size_t entry_signal_delay,
 ErasedSeriesMethod<ErasedSeriesMethodContext> entry_price_method,
 std::size_t exit_signal_delay,
 ErasedSeriesMethod<ErasedSeriesMethodContext> exit_price_method,
 std::size_t pyramiding_signal_delay,
 ErasedSeriesMethod<ErasedSeriesMethodContext> pyramiding_price_method,
 StopTargetReferencePrice favorable_stop_target_reference,
 StopTargetReferencePrice unfavorable_stop_target_reference,
 double signal_exit_reduce,
 double stop_loss_reduce,
 std::vector<BacktestRunner::PositionRule::TakeProfitRule> take_profits,
 ErasedSeriesMethod<ErasedSeriesMethodContext> risk_distance_method)
 -> BacktestRunner::PositionRule
{
  auto signal_exits =
   std::vector<BacktestRunner::PositionRule::SignalExitRule>{};
  signal_exits.emplace_back(true,
                            std::move(exit_method),
                            exit_signal_delay == 0 ? SignalTiming::CurrentClose
                                                   : SignalTiming::NextOpen,
                            signal_exit_reduce);
  auto stop_losses = single_stop_loss(std::move(stop_price_method),
                                      stop_loss_enabled,
                                      stop_loss_trailing_enabled,
                                      stop_loss_reduce);
  return BacktestRunner::PositionRule{std::move(entry_method),
                                      std::move(signal_exits),
                                      std::move(pyramiding_signal),
                                      pyramiding_max_layers,
                                      std::move(risk_distance_method),
                                      std::move(stop_losses),
                                      SignalTiming::CurrentClose,
                                      pyramiding_signal_delay == 0
                                       ? SignalTiming::CurrentClose
                                       : SignalTiming::NextOpen,
                                      favorable_stop_target_reference,
                                      unfavorable_stop_target_reference,
                                      std::move(take_profits)};
}

auto make_position_rule_with_risk_distance(
 ErasedSeriesMethod<ErasedSeriesMethodContext> entry_method,
 ErasedSeriesMethod<ErasedSeriesMethodContext> exit_method,
 ErasedSeriesMethod<ErasedSeriesMethodContext> pyramiding_signal,
 std::size_t pyramiding_max_layers,
 ErasedSeriesMethod<ErasedSeriesMethodContext> risk_distance_method,
 ErasedSeriesMethod<ErasedSeriesMethodContext> stop_price_method,
 bool stop_loss_enabled,
 bool stop_loss_trailing_enabled) -> BacktestRunner::PositionRule
{
  return make_position_rule(std::move(entry_method),
                            std::move(exit_method),
                            std::move(pyramiding_signal),
                            pyramiding_max_layers,
                            std::move(stop_price_method),
                            stop_loss_enabled,
                            stop_loss_trailing_enabled,
                            0,
                            CloseMethod{},
                            0,
                            CloseMethod{},
                            1,
                            OpenMethod{},
                            StopTargetReferencePrice::AveragePrice,
                            StopTargetReferencePrice::AveragePrice,
                            1.0,
                            1.0,
                            {},
                            std::move(risk_distance_method));
}

auto latest_event(const BacktestTimeline& timeline) -> const TradeEvent&
{
  const auto timeline_i = last_timeline_index(timeline);
  return timeline.trade_events(timeline_i).back();
}

TEST(BacktestTimelineTest, DefaultConstructorCreatesEmptyTimeline)
{
  const auto timeline = BacktestTimeline{};

  EXPECT_TRUE(timeline.empty());
  EXPECT_EQ(timeline.size(), 0);
}

TEST(BacktestTimelineTest, AppendsRowsInIndexedColumns)
{
  auto timeline = BacktestTimeline{};
  timeline.append(BacktestTimeline::Row{.market_timestamp = 1,
                                        .market_price = 100.0,
                                        .market_lookback = 3,
                                        .capital = 1000.0,
                                        .equity = 1000.0,
                                        .peak_equity = 1000.0});
  timeline.append(BacktestTimeline::Row{.market_timestamp = 2,
                                        .market_price = 125.0,
                                        .market_lookback = 2,
                                        .capital = 1250.0,
                                        .equity = 1250.0,
                                        .peak_equity = 1250.0});

  ASSERT_EQ(timeline.size(), 2);
  EXPECT_EQ(timeline.market_timestamp(0), 1);
  EXPECT_EQ(timeline.market_timestamp(1), 2);
  EXPECT_DOUBLE_EQ(timeline.market_price(0), 100.0);
  EXPECT_DOUBLE_EQ(timeline.market_price(1), 125.0);
  EXPECT_EQ(timeline.market_lookback(0), 3);
  EXPECT_EQ(timeline.market_lookback(1), 2);
  EXPECT_DOUBLE_EQ(timeline.capital(0), 1000.0);
  EXPECT_DOUBLE_EQ(timeline.capital(1), 1250.0);
  EXPECT_DOUBLE_EQ(timeline.peak_equity(0), 1000.0);
  EXPECT_DOUBLE_EQ(timeline.peak_equity(1), 1250.0);
}

TEST(BacktestTimelineTest, CalculatesDerivedMetricsByIndex)
{
  auto timeline = BacktestTimeline{};
  timeline.append(BacktestTimeline::Row{.capital = 1040.0,
                                        .equity = 1040.0,
                                        .peak_equity = 1040.0,
                                        .cumulative_investment = 400.0,
                                        .profit_count = 1,
                                        .cumulative_profit = 60.0,
                                        .loss_count = 1,
                                        .cumulative_loss = -20.0,
                                        .break_even_count = 2});

  EXPECT_EQ(timeline.trade_count(0), 4);
  EXPECT_DOUBLE_EQ(timeline.cumulative_pnls(0), 40.0);
  EXPECT_DOUBLE_EQ(timeline.average_investment(0), 100.0);
  EXPECT_DOUBLE_EQ(timeline.average_pnl(0), 10.0);
  EXPECT_DOUBLE_EQ(timeline.profit_factor(0), 3.0);
  EXPECT_DOUBLE_EQ(timeline.initial_capital(0), 1000.0);
}

TEST(BacktestTimelineTest, TracksCurrentAndMaximumOutcomeStreaks)
{
  const auto make_trade = [](double pnl) {
    return ClosedTrade{0,
                       0,
                       TradeEvent::Type::exit_signal,
                       0,
                       1,
                       1.0,
                       100.0,
                       100.0,
                       100.0 + pnl,
                       0.0,
                       0.0};
  };

  auto timeline = BacktestTimeline{};
  for(const auto pnl : {10.0, 20.0, -5.0, -10.0, -15.0, 0.0, 5.0}) {
    timeline.append(BacktestTimeline::Row{.closed_trades = {make_trade(pnl)}});
  }

  EXPECT_EQ(timeline.current_winning_streak(), 1);
  EXPECT_EQ(timeline.current_losing_streak(), 0);
  EXPECT_EQ(timeline.maximum_winning_streak(), 2);
  EXPECT_EQ(timeline.maximum_losing_streak(), 3);

  timeline.clear();
  EXPECT_EQ(timeline.current_winning_streak(), 0);
  EXPECT_EQ(timeline.current_losing_streak(), 0);
  EXPECT_EQ(timeline.maximum_winning_streak(), 0);
  EXPECT_EQ(timeline.maximum_losing_streak(), 0);
}

TEST(BacktestRunnerTest, RiskSizingUsesCurrentEquityAfterClosedTrade)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0, 3.0, 4.0}},
                                        {"Open", {50.0, 100.0, 110.0, 110.0}},
                                        {"High", {50.0, 100.0, 110.0, 110.0}},
                                        {"Low", {50.0, 100.0, 110.0, 110.0}},
                                        {"Close", {50.0, 100.0, 110.0, 110.0}},
                                        {"Volume", {0.0, 0.0, 0.0, 0.0}}}};

  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{RiskDistancePositionSizing{0.10}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto entry_signal =
   LogicalOrMethod{EqualMethod{CloseMethod{}, ValueMethod{50.0}},
                   EqualMethod{CloseMethod{}, ValueMethod{110.0}}};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule_with_risk_distance(
                                entry_signal,
                                EqualMethod{CloseMethod{}, ValueMethod{110.0}},
                                BooleanMethod<false>{},
                                1,
                                AbsDiffMethod{OpenMethod{}, ValueMethod{90.0}},
                                ValueMethod{90.0},
                                false,
                                false),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(
   timeline.open_position(last_timeline_index(timeline))->position_size(), 2.5);

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 3);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);
  EXPECT_DOUBLE_EQ(timeline.equity(last_timeline_index(timeline)), 1150.0);

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 4);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(
   timeline.open_position(last_timeline_index(timeline))->position_size(),
   5.75);
}

TEST(BacktestRunnerTest,
     DefaultRejectRecordsOversizedEntryWithoutOpeningPosition)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{20.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  0,
                                                  OpenMethod{},
                                                  0,
                                                  OpenMethod{}),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 1);
  EXPECT_FALSE(timeline.open_position(0).has_value());
  ASSERT_EQ(timeline.trade_events(0).size(), 1);
  const auto& rejected_event = timeline.trade_events(0).front();
  EXPECT_TRUE(rejected_event.is_rejected());
  EXPECT_EQ(rejected_event.trade_id(), std::size_t{0});
  EXPECT_EQ(rejected_event.event_id(), std::size_t{1});
  EXPECT_DOUBLE_EQ(rejected_event.position_size(), 20.0);
  EXPECT_DOUBLE_EQ(rejected_event.price(), 100.0);
  EXPECT_DOUBLE_EQ(timeline.capital(0), 1000.0);
  EXPECT_DOUBLE_EQ(timeline.equity(0), 1000.0);
  EXPECT_EQ(timeline.trade_count(0), 0);
  EXPECT_EQ(timeline.open_trade_count(0), 0);
  ASSERT_EQ(timeline.position_sizing_decisions(0).size(), 1U);
  const auto& sizing = timeline.position_sizing_decisions(0).front();
  EXPECT_EQ(sizing.outcome, PositionSizingDecisionOutcome::InsufficientCash);
  EXPECT_DOUBLE_EQ(*sizing.primary_quantity, 20.0);
  EXPECT_DOUBLE_EQ(*sizing.broker_normalized_quantity, 20.0);
  EXPECT_FALSE(sizing.final_quantity);
}

TEST(BacktestRunnerTest, CapToAvailableCashOpensLargestAffordableOrder)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test",
           PositionSizingNode{FixedQuantityPositionSizing{20.0}},
           DrawdownAdjustment{},
           InsufficientCashPolicy::CapToAvailableCash};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  0,
                                                  OpenMethod{}),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 1);
  ASSERT_TRUE(timeline.open_position(0).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(0)->position_size(), 10.0);
  ASSERT_EQ(timeline.trade_events(0).size(), 1);
  EXPECT_TRUE(timeline.trade_events(0).front().is_entry());
  ASSERT_EQ(timeline.position_sizing_decisions(0).size(), 1U);
  const auto& sizing = timeline.position_sizing_decisions(0).front();
  EXPECT_EQ(sizing.outcome, PositionSizingDecisionOutcome::Executed);
  EXPECT_TRUE(sizing.cash_capped);
  EXPECT_DOUBLE_EQ(*sizing.primary_quantity, 20.0);
  EXPECT_DOUBLE_EQ(*sizing.final_quantity, 10.0);
}

TEST(BacktestRunnerTest, CapToAvailableCashSkipsWhenBelowMarketMinimum)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 11.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test",
           PositionSizingNode{FixedQuantityPositionSizing{20.0}},
           DrawdownAdjustment{},
           InsufficientCashPolicy::CapToAvailableCash};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  0,
                                                  OpenMethod{}),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 1);
  EXPECT_FALSE(timeline.open_position(0).has_value());
  EXPECT_TRUE(timeline.trade_events(0).empty());
  EXPECT_EQ(timeline.trade_count(0), 0);
  ASSERT_EQ(timeline.position_sizing_decisions(0).size(), 1U);
  EXPECT_EQ(timeline.position_sizing_decisions(0).front().outcome,
            PositionSizingDecisionOutcome::InsufficientCash);
}

TEST(BacktestRunnerTest, RejectedPyramidingDoesNotChangePosition)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{6.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<true>{},
                                                  2,
                                                  OpenMethod{},
                                                  false,
                                                  false),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_TRUE(timeline.open_position(1).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 6.0);
  ASSERT_EQ(timeline.trade_events(1).size(), 1);
  const auto& rejected_event = timeline.trade_events(1).front();
  EXPECT_TRUE(rejected_event.is_rejected());
  EXPECT_EQ(rejected_event.trade_id(), std::size_t{1});
  EXPECT_DOUBLE_EQ(rejected_event.position_size(), 6.0);
  EXPECT_DOUBLE_EQ(rejected_event.position_size_before(), 6.0);
  EXPECT_DOUBLE_EQ(rejected_event.position_size_after(), 6.0);
  EXPECT_EQ(timeline.open_trade_count(1), 1);
  EXPECT_EQ(timeline.trade_count(1), 0);
}

TEST(BacktestRunnerTest, EquitySignalUsesCurrentAccountState)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0}},
                                        {"Open", {100.0, 110.0}},
                                        {"High", {100.0, 110.0}},
                                        {"Low", {100.0, 110.0}},
                                        {"Close", {100.0, 110.0}},
                                        {"Volume", {0.0, 0.0}}}};
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  series_methods.set("equity", EquityMethod{});

  const auto entry_signal = EqualMethod{EquityMethod{}, ValueMethod{1000.0}};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               std::move(series_methods),
                               make_position_rule(entry_signal,
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  0,
                                                  OpenMethod{}),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 1);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
  const auto first_results = series_results.results(std::string{"equity"});
  ASSERT_TRUE(first_results.has_value());
  ASSERT_EQ(first_results->get().size(), 1);
  EXPECT_DOUBLE_EQ(first_results->get()[0], 1000.0);

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
  const auto second_results = series_results.results(std::string{"equity"});
  ASSERT_TRUE(second_results.has_value());
  ASSERT_EQ(second_results->get().size(), 2);
  EXPECT_DOUBLE_EQ(second_results->get()[1], 1010.0);
}

TEST(BacktestRunnerTest, EquityPercentUsesCurrentAccountState)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0, 3.0}},
                                        {"Open", {100.0, 100.0, 100.0}},
                                        {"High", {100.0, 200.0, 110.0}},
                                        {"Low", {100.0, 100.0, 100.0}},
                                        {"Close", {100.0, 200.0, 110.0}},
                                        {"Volume", {0.0, 0.0, 0.0}}}};
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  series_methods.set("equity_percent", EquityPercentMethod{});

  const auto entry_signal =
   EqualMethod{EquityPercentMethod{}, ValueMethod{100.0}};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               std::move(series_methods),
                               make_position_rule(entry_signal,
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  0,
                                                  OpenMethod{}),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 1);
  const auto first_results =
   series_results.results(std::string{"equity_percent"});
  ASSERT_TRUE(first_results.has_value());
  ASSERT_EQ(first_results->get().size(), 1);
  EXPECT_DOUBLE_EQ(first_results->get()[0], 100.0);

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  const auto second_results =
   series_results.results(std::string{"equity_percent"});
  ASSERT_TRUE(second_results.has_value());
  ASSERT_EQ(second_results->get().size(), 2);
  EXPECT_DOUBLE_EQ(second_results->get()[1], 110.0);
  EXPECT_DOUBLE_EQ(timeline.equity(last_timeline_index(timeline)), 1100.0);

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 3);
  const auto third_results =
   series_results.results(std::string{"equity_percent"});
  ASSERT_TRUE(third_results.has_value());
  ASSERT_EQ(third_results->get().size(), 3);
  EXPECT_DOUBLE_EQ(third_results->get()[2], 101.0);
}

TEST(BacktestRunnerTest, DrawdownSignalUsesCurrentAccountState)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0, 3.0, 4.0}},
                                        {"Open", {100.0, 100.0, 90.0, 90.0}},
                                        {"High", {100.0, 100.0, 90.0, 90.0}},
                                        {"Low", {100.0, 100.0, 90.0, 90.0}},
                                        {"Close", {100.0, 100.0, 90.0, 90.0}},
                                        {"Volume", {0.0, 0.0, 0.0, 0.0}}}};
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  series_methods.set("drawdown", DrawdownMethod{});

  const auto entry_signal = EqualMethod{CloseMethod{}, ValueMethod{100.0}};
  const auto exit_signal = EqualMethod{CloseMethod{}, ValueMethod{90.0}};
  const auto short_entry_signal =
   EqualMethod{DrawdownMethod{}, ValueMethod{1.0}};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               std::move(series_methods),
                               make_position_rule(entry_signal,
                                                  exit_signal,
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  0,
                                                  OpenMethod{}),
                               make_position_rule(short_entry_signal,
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  0,
                                                  OpenMethod{}),
                               1000.0};

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 1);
  const auto first_results = series_results.results(std::string{"drawdown"});
  ASSERT_TRUE(first_results.has_value());
  ASSERT_EQ(first_results->get().size(), 1);
  EXPECT_DOUBLE_EQ(first_results->get()[0], 0.0);

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 3);
  EXPECT_DOUBLE_EQ(timeline.drawdown(2), 1.0);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
  const auto last_results = series_results.results(std::string{"drawdown"});
  ASSERT_TRUE(last_results.has_value());
  ASSERT_EQ(last_results->get().size(), 3);
  EXPECT_DOUBLE_EQ(last_results->get()[2], 1.0);
}

TEST(BacktestRunnerTest, AccountStateDrawdownUsesEffectiveCurrentPeak)
{
  const auto account_state =
   BacktestAccountState{1000.0, 100.0, 1000.0, 1000.0};

  EXPECT_DOUBLE_EQ(account_state.equity(), 1100.0);
  EXPECT_DOUBLE_EQ(account_state.effective_peak_equity(), 1100.0);
  EXPECT_DOUBLE_EQ(account_state.drawdown(), 0.0);
  EXPECT_DOUBLE_EQ(account_state.drawdown_ratio(), 0.0);
}

TEST(BacktestRunnerTest, MethodContextObservesMutatedAccountState)
{
  const auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  auto default_context = DefaultMethodContext{series_methods, series_results};
  auto account_state = BacktestAccountState{1000.0, 0.0, 1000.0, 1000.0};
  const auto context =
   BacktestMethodContext{default_context, series_methods, account_state};

  EXPECT_DOUBLE_EQ(context.equity(), 1000.0);
  EXPECT_DOUBLE_EQ(context.drawdown(), 0.0);

  account_state.unrealized_pnl(-100.0);

  EXPECT_DOUBLE_EQ(context.equity(), 900.0);
  EXPECT_DOUBLE_EQ(context.equity_percent(), 90.0);
  EXPECT_DOUBLE_EQ(context.drawdown(), 10.0);
}

TEST(BacktestRunnerTest, FixedQuantitySizingUsesExactQuantity)
{
  const auto timeline =
   run_single_entry(PositionSizingNode{FixedQuantityPositionSizing{12.5}},
                    100.0,
                    DrawdownAdjustment{},
                    2000.0);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 12.5);
}

TEST(BacktestRunnerTest, FixedNotionalSizingConvertsByEntryPrice)
{
  const auto timeline =
   run_single_entry(PositionSizingNode{FixedNotionalPositionSizing{250.0}});

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.5);
}

TEST(BacktestRunnerTest, FixedNotionalSizingConvertsBySelectedEntryPrice)
{
  const auto broker = Broker{"Test",
                             {BrokerFee{"Entry Fee",
                                        BrokerFee::FeeType::PercentageNotional,
                                        BrokerFee::FeePosition::LongAndShort,
                                        BrokerFee::FeeTrigger::Entry,
                                        1.0}}};
  const auto timeline = run_single_close_price_entry(
   PositionSizingNode{FixedNotionalPositionSizing{250.0}},
   100.0,
   125.0,
   std::numeric_limits<double>::quiet_NaN(),
   broker);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 125.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).investment(), 252.5);
  EXPECT_DOUBLE_EQ(latest_position(timeline).total_entry_fees(), 2.5);
}

TEST(BacktestRunnerTest, EquityFractionSizingConvertsByCurrentEquity)
{
  const auto timeline =
   run_single_entry(PositionSizingNode{EquityFractionPositionSizing{0.25}});

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.5);
}

TEST(BacktestRunnerTest, EquityFractionSizingConvertsBySelectedEntryPrice)
{
  const auto timeline = run_single_close_price_entry(
   PositionSizingNode{EquityFractionPositionSizing{0.25}});

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 125.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
}

TEST(BacktestRunnerTest, RiskDistanceSizingUsesSelectedEntryPrice)
{
  const auto timeline = run_single_close_price_entry(
   PositionSizingNode{RiskDistancePositionSizing{0.10}},
   100.0,
   125.0,
   80.0,
   Broker{"Test"},
   50.0);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 125.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   80.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).risk_distance(), 50.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).risk_reference_price(), 125.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).risk_boundary_price(), 75.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).effective_price(),
                   80.0);
  EXPECT_FALSE(stop_level(latest_position(timeline)).active());
}

TEST(BacktestRunnerTest, DisabledTakeProfitKeepsTargetReferencePrice)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<false>{},
                      1,
                      ValueMethod{90.0},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(ValueMethod{120.0}, false)),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   90.0);
  ASSERT_EQ(latest_position(timeline).take_profit_levels().size(), 1);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 120.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).effective_price(),
                   90.0);
  EXPECT_FALSE(stop_level(latest_position(timeline)).active());
  EXPECT_FALSE(
   latest_position(timeline).take_profit_levels().front().enabled());
}

TEST(BacktestRunnerTest, StopTargetAmountMethodsUseEntryDirection)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<false>{},
                      1,
                      SlAmountMethod{10.0},
                      true,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(TpAmountMethod{20.0}, true)),
   1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 100.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), -1.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   110.0);
  ASSERT_EQ(latest_position(timeline).take_profit_levels().size(), 1);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 80.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).effective_price(),
                   110.0);
  EXPECT_TRUE(latest_position(timeline).take_profit_levels().front().active());
}

TEST(BacktestRunnerTest, ScopedStopTargetAmountMethodsEvaluateDirectly)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto snapshot = asset.get_snapshot(0);
  const auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  auto default_context = DefaultMethodContext{series_methods, series_results};
  const auto account_state = BacktestAccountState{1000.0, 0.0, 1000.0, 1000.0};
  auto context =
   BacktestMethodContext{default_context, series_methods, account_state};

  const auto long_context = context.with_position_reference(100.0, 1.0);
  const auto short_context = context.with_position_reference(100.0, -1.0);

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(SlAmountMethod{10.0}, snapshot, long_context), 90.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(TpAmountMethod{20.0}, snapshot, long_context), 120.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(SlAmountMethod{10.0}, snapshot, short_context),
   110.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(TpAmountMethod{20.0}, snapshot, short_context), 80.0);
}

TEST(BacktestRunnerTest, PositionContextMethodsEvaluateDirectly)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto snapshot = asset.get_snapshot(0);
  const auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  auto default_context = DefaultMethodContext{series_methods, series_results};
  const auto account_state = BacktestAccountState{1000.0, 0.0, 1000.0, 1000.0};
  auto context =
   BacktestMethodContext{default_context, series_methods, account_state};
  const auto scoped_context =
   context.with_position_prices(90.0, 120.0, 105.0, 110.0, -1.0);
  const auto risk_context = scoped_context.with_position_risk_distance(10.0);
  const auto long_risk_context =
   context.with_position_prices(100.0, 100.0, 100.0, 100.0, 1.0)
    .with_position_risk_distance(10.0);

  EXPECT_TRUE(std::isnan(
   evaluate_series_method(InitialEntryPriceMethod{}, snapshot, context)));
  EXPECT_TRUE(std::isnan(
   evaluate_series_method(LatestEntryPriceMethod{}, snapshot, context)));
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(AveragePriceMethod{}, snapshot, context)));
  EXPECT_TRUE(std::isnan(
   evaluate_series_method(StopTargetRefPriceMethod{}, snapshot, context)));
  EXPECT_TRUE(std::isnan(
   evaluate_series_method(PositionDirectionMethod{}, snapshot, context)));

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(InitialEntryPriceMethod{}, snapshot, scoped_context),
   90.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(LatestEntryPriceMethod{}, snapshot, scoped_context),
   120.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(AveragePriceMethod{}, snapshot, scoped_context),
   105.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(StopTargetRefPriceMethod{}, snapshot, scoped_context),
   110.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(PositionDirectionMethod{}, snapshot, scoped_context),
   -1.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(PositionRMultipleMethod{}, snapshot, risk_context),
   1.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(
    PositionRMultipleMethod{ValueMethod{125.0}}, snapshot, long_risk_context),
   2.5);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(
    PositionRMultipleMethod{ValueMethod{95.0}}, snapshot, long_risk_context),
   -0.5);
  EXPECT_TRUE(std::isnan(evaluate_series_method(
   PositionRMultipleMethod{}, snapshot, scoped_context)));
  EXPECT_TRUE(std::isnan(
   evaluate_series_method(PositionRMultipleMethod{}, snapshot, context)));
}

TEST(BacktestRunnerTest, StopTargetPercentageMethodsUseEntryPrice)
{
  const auto asset = make_single_bar_asset(200.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<false>{},
                      1,
                      SlPercentMethod{10.0},
                      true,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(TpPercentMethod{20.0}, true)),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   180.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 240.0);
}

TEST(BacktestRunnerTest, StopTargetPercentageMethodsUseRawShadowAveragePrice)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test",
                             {BrokerFee{"Entry Fee",
                                        BrokerFee::FeeType::Fixed,
                                        BrokerFee::FeePosition::LongAndShort,
                                        BrokerFee::FeeTrigger::Entry,
                                        10.0}}};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<false>{},
                      1,
                      SlPercentMethod{10.0},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(TpRMultipleMethod{2.0}, false),
                      RiskDistancePercentMethod{10.0}),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   90.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 120.0);
}

TEST(BacktestRunnerTest, ScopedStopTargetPercentMethodsEvaluateDirectly)
{
  const auto asset = make_single_bar_asset(200.0);
  const auto snapshot = asset.get_snapshot(0);
  const auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  auto default_context = DefaultMethodContext{series_methods, series_results};
  const auto account_state = BacktestAccountState{1000.0, 0.0, 1000.0, 1000.0};
  const auto context =
   BacktestMethodContext{default_context, series_methods, account_state}
    .with_position_reference(200.0, 1.0);

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(SlPercentMethod{10.0}, snapshot, context), 180.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(TpPercentMethod{20.0}, snapshot, context), 240.0);
}

TEST(BacktestRunnerTest, AtrStopAndRMultipleTargetUseScopedContext)
{
  const auto asset =
   make_single_bar_asset_with_range(100.0, 110.0, 90.0, 100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<false>{},
                      1,
                      SlAtrMethod{1.0, 2.0},
                      true,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(TpRMultipleMethod{2.0}, true),
                      RiskDistanceAtrMethod{1.0, 2.0}),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 100.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   60.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 180.0);
}

TEST(BacktestRunnerTest, PositionContextMethodsUseNormalLongEntryContext)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<false>{},
                      1,
                      InitialEntryPriceMethod{},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(PositionDirectionMethod{}, false)),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   100.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 1.0);
}

TEST(BacktestRunnerTest, PositionContextMethodsUseNormalShortEntryContext)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<false>{},
                      1,
                      LatestEntryPriceMethod{},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(PositionDirectionMethod{}, false)),
   1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   100.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), -1.0);
}

TEST(BacktestRunnerTest, PositionContextMethodsUsePyramidingPriceContext)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 120.0, 120.0, 120.0, 120.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<true>{},
                      2,
                      AveragePriceMethod{},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(LatestEntryPriceMethod{}, false)),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   110.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 120.0);
}

TEST(BacktestRunnerTest, PositionContextMethodsUseConfiguredReferenceContext)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 120.0, 120.0, 120.0, 120.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<true>{},
                      2,
                      InitialEntryPriceMethod{},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::LatestEntryPrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(StopTargetRefPriceMethod{}, false)),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   100.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 120.0);
}

TEST(BacktestRunnerTest, CustomStopTargetFormulasUseReferenceAndDirection)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto stop_method =
   SubtractMethod{StopTargetRefPriceMethod{},
                  MultiplyMethod{PositionDirectionMethod{}, ValueMethod{10.0}}};
  const auto target_method =
   AddMethod{StopTargetRefPriceMethod{},
             MultiplyMethod{PositionDirectionMethod{}, ValueMethod{20.0}}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  make_position_rule(BooleanMethod<true>{},
                                     BooleanMethod<false>{},
                                     BooleanMethod<false>{},
                                     1,
                                     stop_method,
                                     false,
                                     false,
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     1.0,
                                     1.0,
                                     single_take_profit(target_method, false)),
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   90.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 120.0);
}

TEST(BacktestRunnerTest, ExplicitRiskDistanceAndRPricesEvaluateDirectly)
{
  const auto asset =
   make_single_bar_asset_with_range(100.0, 110.0, 90.0, 100.0);
  const auto snapshot = asset.get_snapshot(0);
  const auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  auto default_context = DefaultMethodContext{series_methods, series_results};
  const auto account_state = BacktestAccountState{1000.0, 0.0, 1000.0, 1000.0};
  auto context =
   BacktestMethodContext{default_context, series_methods, account_state};
  const auto long_context = context.with_position_reference(100.0, 1.0);
  const auto risk_distance = evaluate_series_method(
   RiskDistanceAtrMethod{1.0, 2.0}, snapshot, long_context);
  const auto long_r_context =
   long_context.with_position_risk_distance(risk_distance);

  EXPECT_DOUBLE_EQ(risk_distance, 40.0);
  EXPECT_DOUBLE_EQ(evaluate_series_method(
                    RiskDistanceAmountMethod{10.0}, snapshot, long_context),
                   10.0);
  EXPECT_DOUBLE_EQ(evaluate_series_method(
                    RiskDistancePercentMethod{5.0}, snapshot, long_context),
                   5.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(Sl1RMethod{}, snapshot, long_r_context), 60.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(TpRMultipleMethod{2.0}, snapshot, long_r_context),
   180.0);

  const auto short_r_context = context.with_position_reference(100.0, -1.0)
                                .with_position_risk_distance(risk_distance);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(Sl1RMethod{}, snapshot, short_r_context), 140.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(TpRMultipleMethod{2.0}, snapshot, short_r_context),
   20.0);

  EXPECT_TRUE(std::isnan(
   evaluate_series_method(TpRMultipleMethod{-1.0}, snapshot, long_r_context)));
  EXPECT_TRUE(std::isnan(evaluate_series_method(
   Sl1RMethod{}, snapshot, long_context.with_position_risk_distance(0.0))));
  EXPECT_TRUE(std::isnan(
   evaluate_series_method(TpRMultipleMethod{1.0},
                          snapshot,
                          long_context.with_position_risk_distance(0.0))));
}

TEST(BacktestRunnerTest, RejectsInvalidExplicitRiskDistanceForEverySizingMode)
{
  const auto invalid_distances =
   std::array{0.0,
              -1.0,
              std::numeric_limits<double>::quiet_NaN(),
              std::numeric_limits<double>::infinity()};

  for(const auto invalid_distance : invalid_distances) {
    const auto asset = make_single_bar_asset(100.0);
    const auto market = Market{"Test", 0.0, 0.0};
    const auto broker = Broker{"Test"};
    const auto profile =
     Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
    auto series_results = SeriesEvaluationResults{};
    auto timeline = BacktestTimeline{};
    auto runner = BacktestRunner{
     asset,
     market,
     broker,
     profile,
     {},
     make_position_rule_with_risk_distance(BooleanMethod<true>{},
                                           BooleanMethod<false>{},
                                           BooleanMethod<false>{},
                                           1,
                                           ValueMethod{invalid_distance},
                                           ValueMethod{90.0},
                                           false,
                                           false),
     BacktestRunner::PositionRule{},
     1000.0};

    EXPECT_THROW(runner.run(series_results, timeline), std::runtime_error);
  }
}

TEST(BacktestRunnerTest,
     PyramidingSizesNewLayerFromNormalizedShadowRiskDistance)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 200.0, 200.0, 200.0, 200.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{RiskDistancePositionSizing{0.01}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  make_position_rule(BooleanMethod<true>{},
                                     BooleanMethod<false>{},
                                     BooleanMethod<true>{},
                                     2,
                                     ValueMethod{90.0},
                                     false,
                                     false,
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     1.0,
                                     1.0,
                                     {},
                                     RiskDistancePercentMethod{10.0}),
                  BacktestRunner::PositionRule{},
                  10000.0};

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(0));
  EXPECT_DOUBLE_EQ(timeline.open_position(0)->position_size(), 10.0);
  EXPECT_DOUBLE_EQ(timeline.open_position(0)->risk_distance(), 10.0);

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).front().position_size(), 5.5);
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 15.5);
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->risk_reference_price(), 150.0);
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->risk_distance(), 15.0);
}

TEST(BacktestRunnerTest, DisabledDrawdownAdjustmentLeavesSizingUnchanged)
{
  const auto timeline =
   run_single_entry(PositionSizingNode{FixedQuantityPositionSizing{100.0}},
                    100.0,
                    DrawdownAdjustment{false, 0.10, 0.20},
                    90000.0,
                    100000.0);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 100.0);
}

TEST(BacktestRunnerTest, DrawdownAdjustmentLeavesSizeUnchangedAtZeroDrawdown)
{
  const auto timeline =
   run_single_entry(PositionSizingNode{FixedQuantityPositionSizing{100.0}},
                    100.0,
                    DrawdownAdjustment{true, 0.10, 0.20},
                    10000.0);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 100.0);
}

TEST(BacktestRunnerTest, DrawdownAdjustmentReducesSizeAtTenPercentDrawdown)
{
  const auto timeline =
   run_single_entry(PositionSizingNode{FixedQuantityPositionSizing{100.0}},
                    100.0,
                    DrawdownAdjustment{true, 0.10, 0.20},
                    9000.0,
                    10000.0);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 80.0);
}

TEST(BacktestRunnerTest, DrawdownAdjustmentReducesSizeAtTwentyPercentDrawdown)
{
  const auto timeline =
   run_single_entry(PositionSizingNode{FixedQuantityPositionSizing{100.0}},
                    100.0,
                    DrawdownAdjustment{true, 0.10, 0.20},
                    8000.0,
                    10000.0);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 60.0);
}

TEST(BacktestRunnerTest, DrawdownAdjustmentAtZeroSkipsExecution)
{
  const auto timeline =
   run_single_entry(PositionSizingNode{FixedQuantityPositionSizing{100.0}},
                    100.0,
                    DrawdownAdjustment{true, 0.10, 0.20},
                    400.0,
                    1000.0);

  const auto index = last_timeline_index(timeline);
  EXPECT_FALSE(timeline.open_position(index).has_value());
  ASSERT_EQ(timeline.position_sizing_decisions(index).size(), 1U);
  const auto& decision = timeline.position_sizing_decisions(index).front();
  EXPECT_EQ(decision.outcome,
            PositionSizingDecisionOutcome::DrawdownSuppressed);
  EXPECT_DOUBLE_EQ(*decision.primary_quantity, 100.0);
  EXPECT_DOUBLE_EQ(*decision.drawdown_adjusted_quantity, 0.0);
}

auto make_two_bar_asset(double first_open,
                        double first_high,
                        double first_low,
                        double first_close,
                        double second_open,
                        double second_high,
                        double second_low,
                        double second_close) -> Asset
{
  return Asset{"Test",
               AssetHistory{{"Datetime", {1.0, 2.0}},
                            {"Open", {first_open, second_open}},
                            {"High", {first_high, second_high}},
                            {"Low", {first_low, second_low}},
                            {"Close", {first_close, second_close}},
                            {"Volume", {0.0, 0.0}}}};
}

auto make_three_bar_asset(double first_open,
                          double first_high,
                          double first_low,
                          double first_close,
                          double second_open,
                          double second_high,
                          double second_low,
                          double second_close,
                          double third_open,
                          double third_high,
                          double third_low,
                          double third_close) -> Asset
{
  return Asset{"Test",
               AssetHistory{{"Datetime", {1.0, 2.0, 3.0}},
                            {"Open", {first_open, second_open, third_open}},
                            {"High", {first_high, second_high, third_high}},
                            {"Low", {first_low, second_low, third_low}},
                            {"Close", {first_close, second_close, third_close}},
                            {"Volume", {0.0, 0.0, 0.0}}}};
}

TEST(BacktestRunnerTest, PositionRMultipleTriggersCurrentCloseSignalExit)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 125.0, 125.0, 125.0, 125.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto exit_signal =
   GreaterEqualMethod{PositionRMultipleMethod{}, ValueMethod{2.0}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  make_position_rule(BooleanMethod<true>{},
                                     exit_signal,
                                     BooleanMethod<false>{},
                                     1,
                                     OpenMethod{},
                                     false,
                                     false,
                                     1,
                                     OpenMethod{},
                                     0,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     1.0,
                                     1.0,
                                     {},
                                     ValueMethod{10.0}),
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.closed_trades(1).size(), 1U);
  EXPECT_DOUBLE_EQ(timeline.closed_trades(1).front().exit_price(), 125.0);
}

TEST(BacktestRunnerTest, PositionRMultipleSchedulesNextOpenSignalExit)
{
  const auto asset = make_three_bar_asset(100.0,
                                          100.0,
                                          100.0,
                                          100.0,
                                          125.0,
                                          125.0,
                                          125.0,
                                          125.0,
                                          130.0,
                                          130.0,
                                          130.0,
                                          130.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto exit_signal =
   GreaterEqualMethod{PositionRMultipleMethod{}, ValueMethod{2.0}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  make_position_rule(BooleanMethod<true>{},
                                     exit_signal,
                                     BooleanMethod<false>{},
                                     1,
                                     OpenMethod{},
                                     false,
                                     false,
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     1.0,
                                     1.0,
                                     {},
                                     ValueMethod{10.0}),
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.closed_trades(2).size(), 1U);
  EXPECT_DOUBLE_EQ(timeline.closed_trades(2).front().exit_price(), 130.0);
}

TEST(BacktestRunnerTest, PositionRMultipleDrivesCurrentClosePyramiding)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 125.0, 125.0, 125.0, 125.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto pyramiding_signal =
   GreaterEqualMethod{PositionRMultipleMethod{}, ValueMethod{2.0}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  make_position_rule(BooleanMethod<true>{},
                                     BooleanMethod<false>{},
                                     pyramiding_signal,
                                     2,
                                     OpenMethod{},
                                     false,
                                     false,
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     0,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     1.0,
                                     1.0,
                                     {},
                                     ValueMethod{10.0}),
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(timeline.open_position(1).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 2.0);
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->average_price(), 112.5);
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->risk_reference_price(), 112.5);
}

TEST(BacktestRunnerTest, PositionRMultipleIsCachedForNamedSeriesAndPlots)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 125.0, 125.0, 125.0, 125.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  series_methods.set("current_r", PositionRMultipleMethod{});

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  std::move(series_methods),
                  make_position_rule(BooleanMethod<true>{},
                                     BooleanMethod<false>{},
                                     BooleanMethod<false>{},
                                     1,
                                     OpenMethod{},
                                     false,
                                     false,
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     1.0,
                                     1.0,
                                     {},
                                     ValueMethod{10.0}),
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  const auto values = series_results.results(std::string{"current_r"});
  ASSERT_TRUE(values.has_value());
  ASSERT_EQ(values->get().size(), 2U);
  EXPECT_DOUBLE_EQ(values->get()[0], 0.0);
  EXPECT_DOUBLE_EQ(values->get()[1], 2.5);
}

TEST(BacktestRunnerTest, SeriesDelayedSignalUsesCompletedResultsOnly)
{
  const auto asset = make_three_bar_asset(100.0,
                                          100.0,
                                          100.0,
                                          10.0,
                                          100.0,
                                          100.0,
                                          100.0,
                                          20.0,
                                          100.0,
                                          100.0,
                                          100.0,
                                          30.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  series_methods.set("close", CloseMethod{});

  const auto entry_signal =
   EqualMethod{SeriesMethod{"close"}, ValueMethod{20.0}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  std::move(series_methods),
                  BacktestRunner::PositionRule{entry_signal,
                                               {},
                                               BooleanMethod<false>{},
                                               1,
                                               ValueMethod{10.0},
                                               {},
                                               SignalTiming::NextOpen},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 1);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);
  auto close_results = series_results.results(std::string{"close"});
  ASSERT_TRUE(close_results.has_value());
  ASSERT_EQ(close_results->get().size(), 1);
  EXPECT_DOUBLE_EQ(close_results->get()[0], 10.0);

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);
  close_results = series_results.results(std::string{"close"});
  ASSERT_TRUE(close_results.has_value());
  ASSERT_EQ(close_results->get().size(), 2);
  EXPECT_DOUBLE_EQ(close_results->get()[1], 20.0);

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 3);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
  close_results = series_results.results(std::string{"close"});
  ASSERT_TRUE(close_results.has_value());
  ASSERT_EQ(close_results->get().size(), 3);
  EXPECT_DOUBLE_EQ(close_results->get()[2], 30.0);
}

TEST(BacktestRunnerTest, SeriesDelayZeroSignalEvaluatesCurrentBar)
{
  const auto asset = make_single_bar_asset_with_close(100.0, 125.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  series_methods.set("close", CloseMethod{});

  const auto entry_signal =
   EqualMethod{SeriesMethod{"close"}, ValueMethod{125.0}};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               std::move(series_methods),
                               make_position_rule(entry_signal,
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  0,
                                                  OpenMethod{}),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 1);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
  const auto close_results = series_results.results(std::string{"close"});
  ASSERT_TRUE(close_results.has_value());
  ASSERT_EQ(close_results->get().size(), 1);
  EXPECT_DOUBLE_EQ(close_results->get()[0], 125.0);
}

TEST(BacktestRunnerTest,
     SignalDelayZeroCloseEntryUsesCurrentSignalAndClosePrice)
{
  const auto asset = make_single_bar_asset_with_close(100.0, 125.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedNotionalPositionSizing{250.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  0,
                                                  CloseMethod{}),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 125.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
}

TEST(BacktestRunnerTest, NextOpenEntryUsesPreviousSignalAndOpenPrice)
{
  const auto asset = make_three_bar_asset(100.0,
                                          100.0,
                                          100.0,
                                          0.0,
                                          110.0,
                                          110.0,
                                          110.0,
                                          75.0,
                                          125.0,
                                          125.0,
                                          125.0,
                                          0.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto entry_signal = EqualMethod{CloseMethod{}, ValueMethod{75.0}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{entry_signal,
                                               {},
                                               BooleanMethod<false>{},
                                               1,
                                               ValueMethod{10.0},
                                               {},
                                               SignalTiming::NextOpen},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 125.0);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
}

TEST(BacktestRunnerTest, SignalDelayZeroEntryUsesCurrentSignal)
{
  const auto asset = make_three_bar_asset(100.0,
                                          100.0,
                                          100.0,
                                          0.0,
                                          110.0,
                                          110.0,
                                          110.0,
                                          75.0,
                                          125.0,
                                          125.0,
                                          125.0,
                                          0.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto entry_signal = EqualMethod{CloseMethod{}, ValueMethod{75.0}};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(entry_signal,
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  0,
                                                  CloseMethod{}),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 75.0);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
}

TEST(BacktestRunnerTest, SignalDelayZeroCloseExitUsesCurrentSignalAndClosePrice)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 110.0, 120.0, 110.0, 120.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto exit_signal = EqualMethod{CloseMethod{}, ValueMethod{120.0}};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  exit_signal,
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  1,
                                                  OpenMethod{},
                                                  0,
                                                  CloseMethod{}),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.closed_trades(last_timeline_index(timeline)).size(), 1);
  EXPECT_EQ(latest_closed_trade(timeline).exit_type(),
            TradeEvent::Type::exit_signal);
  EXPECT_DOUBLE_EQ(latest_closed_trade(timeline).exit_price(), 120.0);
}

TEST(BacktestRunnerTest, SignalDelayZeroClosePyramidingUsesOwnTimingAndPrice)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 110.0, 120.0, 110.0, 120.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto pyramiding_signal = EqualMethod{CloseMethod{}, ValueMethod{120.0}};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  pyramiding_signal,
                                                  2,
                                                  OpenMethod{},
                                                  false,
                                                  false,
                                                  1,
                                                  OpenMethod{},
                                                  1,
                                                  OpenMethod{},
                                                  0,
                                                  CloseMethod{}),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).investment(), 220.0);
}

TEST(BacktestRunnerTest,
     FavorableLongPyramidingUsesPostActionAverageReferenceByDefault)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 120.0, 120.0, 120.0, 120.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<true>{},
                      2,
                      SlPercentMethod{10.0},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(TpRMultipleMethod{2.0}, false),
                      RiskDistancePercentMethod{10.0}),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   99.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 132.0);
}

TEST(BacktestRunnerTest,
     PyramidingUsesPostScaleNormalizedShadowAverageReference)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 120.0, 120.0, 120.0, 120.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test",
                             {BrokerFee{"Entry Fee",
                                        BrokerFee::FeeType::Fixed,
                                        BrokerFee::FeePosition::LongAndShort,
                                        BrokerFee::FeeTrigger::Entry,
                                        10.0}}};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<true>{},
                      2,
                      SlPercentMethod{10.0},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(TpRMultipleMethod{2.0}, false),
                      RiskDistancePercentMethod{10.0}),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 120.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   99.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 132.0);
}

TEST(BacktestRunnerTest,
     UnfavorableLongPyramidingUsesPostActionAverageReferenceByDefault)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 80.0, 80.0, 80.0, 80.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<true>{},
                      2,
                      SlPercentMethod{10.0},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(TpRMultipleMethod{2.0}, false),
                      RiskDistancePercentMethod{10.0}),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 90.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   81.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 108.0);
}

TEST(BacktestRunnerTest,
     FavorableShortPyramidingUsesPostActionAverageReferenceByDefault)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 80.0, 80.0, 80.0, 80.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<true>{},
                      2,
                      SlPercentMethod{10.0},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(TpRMultipleMethod{2.0}, false),
                      RiskDistancePercentMethod{10.0}),
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), -2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 90.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   99.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 72.0);
}

TEST(BacktestRunnerTest, PyramidingCanUseLatestEntryReference)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 120.0, 120.0, 120.0, 120.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<true>{},
                      2,
                      SlPercentMethod{10.0},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::LatestEntryPrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(TpRMultipleMethod{2.0}, false),
                      RiskDistancePercentMethod{10.0}),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   108.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 144.0);
}

TEST(BacktestRunnerTest,
     UnfavorableShortPyramidingUsesPostActionAverageReferenceByDefault)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 120.0, 120.0, 120.0, 120.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<true>{},
                      2,
                      SlPercentMethod{10.0},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(TpRMultipleMethod{2.0}, false),
                      RiskDistancePercentMethod{10.0}),
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), -2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   121.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 88.0);
}

TEST(BacktestRunnerTest, PyramidingCanUseInitialEntryReference)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 120.0, 120.0, 120.0, 120.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<true>{},
                      2,
                      SlPercentMethod{10.0},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::InitialEntryPrice,
                      StopTargetReferencePrice::InitialEntryPrice,
                      1.0,
                      1.0,
                      single_take_profit(TpRMultipleMethod{2.0}, false),
                      RiskDistancePercentMethod{10.0}),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_position(timeline)).evaluated_price(),
                   90.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 120.0);
}

TEST(BacktestRunnerTest, SameBarSameDirectionReentryIsBlockedAfterExit)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 110.0, 110.0, 110.0, 110.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);
  ASSERT_EQ(timeline.closed_trades(last_timeline_index(timeline)).size(), 1);
  EXPECT_EQ(latest_closed_trade(timeline).exit_type(),
            TradeEvent::Type::exit_signal);
}

TEST(BacktestRunnerTest, PyramidingIsSkippedOnExitBar)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 110.0, 110.0, 110.0, 110.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<true>{},
                                                  BooleanMethod<true>{},
                                                  2,
                                                  OpenMethod{},
                                                  false,
                                                  false),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);
  ASSERT_EQ(timeline.closed_trades(last_timeline_index(timeline)).size(), 1);
  ASSERT_EQ(timeline.trade_events(last_timeline_index(timeline)).size(), 1);
  const auto& exit_event = latest_event(timeline);
  EXPECT_TRUE(exit_event.is_exit());
  EXPECT_FALSE(exit_event.is_scale_in());
  EXPECT_DOUBLE_EQ(latest_closed_trade(timeline).position_size(), 1.0);
}

TEST(BacktestRunnerTest, SameBarOppositeDirectionReversalIsAllowedAfterExit)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 110.0, 110.0, 110.0, 110.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false),
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  OpenMethod{},
                                                  false,
                                                  false),
                               1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
  ASSERT_EQ(timeline.closed_trades(last_timeline_index(timeline)).size(), 1);
  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_EQ(
   timeline.closed_trades(last_timeline_index(timeline)).front().exit_type(),
   TradeEvent::Type::exit_signal);
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), -1.0);
}

TEST(BacktestRunnerTest, StopLossExitIsDecidedByRunner)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 95.0, 96.0, 89.0, 95.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  ValueMethod{90.0},
                                                  true,
                                                  false),
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_EQ(timeline.closed_trades(last_timeline_index(timeline)).size(), 1);
  EXPECT_EQ(latest_closed_trade(timeline).exit_type(),
            TradeEvent::Type::stop_loss);
  EXPECT_DOUBLE_EQ(stop_level(latest_closed_trade(timeline)).evaluated_price(),
                   90.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_closed_trade(timeline)).effective_price(),
                   90.0);
  EXPECT_DOUBLE_EQ(latest_closed_trade(timeline).exit_price(), 90.0);
}

TEST(BacktestRunnerTest, TrailingStopMutationUsesTradePositionState)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 100.0, 120.0, 105.0, 110.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<false>{},
                                                  1,
                                                  ValueMethod{90.0},
                                                  true,
                                                  true),
                               BacktestRunner::PositionRule{},
                               1000.0,
                               0,
                               false,
                               NAN,
                               IntrabarPath::HighFirst};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_EQ(timeline.closed_trades(last_timeline_index(timeline)).size(), 1);
  EXPECT_EQ(latest_closed_trade(timeline).exit_type(),
            TradeEvent::Type::stop_loss);
  EXPECT_DOUBLE_EQ(stop_level(latest_closed_trade(timeline)).evaluated_price(),
                   90.0);
  EXPECT_DOUBLE_EQ(stop_level(latest_closed_trade(timeline)).effective_price(),
                   110.0);
  EXPECT_DOUBLE_EQ(latest_closed_trade(timeline).exit_price(), 110.0);
}

TEST(BacktestRunnerTest,
     SimultaneousStopLossesFillInPathOrderAndResetAfterClosure)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0, 3.0, 4.0}},
                                        {"Open", {100.0, 85.0, 75.0, 100.0}},
                                        {"High", {100.0, 100.0, 80.0, 100.0}},
                                        {"Low", {100.0, 75.0, 70.0, 100.0}},
                                        {"Close", {100.0, 85.0, 75.0, 100.0}},
                                        {"Volume", {0.0, 0.0, 0.0, 0.0}}}};
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto stop_losses = std::vector<BacktestRunner::PositionRule::StopLossRule>{};
  stop_losses.emplace_back(ValueMethod{95.0}, false, false, 1.0);
  stop_losses.emplace_back(ValueMethod{90.0}, true, false, 0.5);
  stop_losses.emplace_back(ValueMethod{80.0}, true, false, 1.0);

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               {},
                                               BooleanMethod<false>{},
                                               1,
                                               ValueMethod{10.0},
                                               std::move(stop_losses),
                                               SignalTiming::CurrentClose,
                                               SignalTiming::CurrentClose},
                  BacktestRunner::PositionRule{},
                  5000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  EXPECT_FALSE(timeline.open_position(1));
  ASSERT_EQ(timeline.trade_events(1).size(), 2);
  EXPECT_EQ(timeline.trade_events(1).front().type(),
            TradeEvent::Type::stop_loss);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).front().price(), 85.0);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).back().price(), 80.0);
  ASSERT_EQ(timeline.closed_trades(1).size(), 1);
  EXPECT_TRUE(
   timeline.closed_trades(1).front().stop_loss_levels()[1].consumed());
  EXPECT_TRUE(
   timeline.closed_trades(1).front().stop_loss_levels()[2].consumed());
  EXPECT_DOUBLE_EQ(timeline.closed_trades(1).front().exit_price(), 80.0);

  runner.run(series_results, timeline);

  ASSERT_TRUE(timeline.open_position(2));
  EXPECT_FALSE(timeline.open_position(2)->stop_loss_levels()[1].consumed());
  EXPECT_FALSE(timeline.open_position(2)->stop_loss_levels()[2].consumed());
}

TEST(BacktestRunnerTest, SimultaneousShortStopsUseGapThenPathPrices)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 115.0, 125.0, 100.0, 115.0);
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto stop_losses = std::vector<BacktestRunner::PositionRule::StopLossRule>{};
  stop_losses.emplace_back(ValueMethod{110.0}, true, false, 0.5);
  stop_losses.emplace_back(ValueMethod{120.0}, true, false, 1.0);

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               {},
                                               BooleanMethod<false>{},
                                               1,
                                               ValueMethod{10.0},
                                               std::move(stop_losses)},
                  5000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  EXPECT_FALSE(timeline.open_position(1));
  ASSERT_EQ(timeline.trade_events(1).size(), 2);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).front().price(), 115.0);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).back().price(), 120.0);
}

TEST(BacktestRunnerTest, PyramidingPreservesConsumedStopLossLevels)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0, 3.0}},
                                        {"Open", {100.0, 95.0, 120.0}},
                                        {"High", {100.0, 100.0, 120.0}},
                                        {"Low", {100.0, 89.0, 100.0}},
                                        {"Close", {100.0, 95.0, 120.0}},
                                        {"Volume", {0.0, 0.0, 0.0}}}};
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto stop_losses = std::vector<BacktestRunner::PositionRule::StopLossRule>{};
  stop_losses.emplace_back(ValueMethod{90.0}, true, false, 0.5);
  stop_losses.emplace_back(ValueMethod{80.0}, true, false, 1.0);

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               {},
                                               BooleanMethod<true>{},
                                               2,
                                               ValueMethod{10.0},
                                               std::move(stop_losses),
                                               SignalTiming::CurrentClose,
                                               SignalTiming::CurrentClose},
                  BacktestRunner::PositionRule{},
                  5000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_TRUE(timeline.open_position(1)->stop_loss_levels()[0].consumed());

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(2));
  EXPECT_DOUBLE_EQ(timeline.open_position(2)->position_size(), 12.0);
  EXPECT_TRUE(timeline.open_position(2)->stop_loss_levels()[0].consumed());
  EXPECT_FALSE(timeline.open_position(2)->stop_loss_levels()[1].consumed());
}

TEST(BacktestRunnerTest, PyramidingNeverLoosensTrailingStopEffectivePrice)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0, 3.0}},
                                        {"Open", {100.0, 100.0, 120.0}},
                                        {"High", {100.0, 121.0, 120.0}},
                                        {"Low", {100.0, 100.0, 115.0}},
                                        {"Close", {100.0, 120.0, 120.0}},
                                        {"Volume", {0.0, 0.0, 0.0}}}};
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<true>{},
                      2,
                      SlAmountMethod{10.0},
                      true,
                      true,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(ValueMethod{110.0}, true, 0.5)),
   BacktestRunner::PositionRule{},
   5000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_TRUE(timeline.open_position(1)->take_profit_levels()[0].consumed());

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(2));
  ASSERT_EQ(timeline.open_position(2)->stop_loss_levels().size(), 1);
  EXPECT_DOUBLE_EQ(
   timeline.open_position(2)->stop_loss_levels()[0].evaluated_price(), 90.0);
  EXPECT_DOUBLE_EQ(
   timeline.open_position(2)->stop_loss_levels()[0].effective_price(), 111.0);
}

TEST(BacktestRunnerTest, PyramidingAcceptsSaferTrailingStopBasePrice)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 140.0, 140.0, 120.0, 140.0);
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_position_rule(BooleanMethod<true>{},
                                                  BooleanMethod<false>{},
                                                  BooleanMethod<true>{},
                                                  2,
                                                  SlAmountMethod{10.0},
                                                  true,
                                                  true),
                               BacktestRunner::PositionRule{},
                               5000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  EXPECT_FALSE(timeline.open_position(1));
  ASSERT_EQ(timeline.trade_events(1).size(), 2);
  EXPECT_EQ(timeline.trade_events(1).back().type(),
            TradeEvent::Type::stop_loss);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).back().price(), 130.0);
}

TEST(BacktestRunnerTest, TakeProfitExitIsDecidedByRunner)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 110.0, 121.0, 109.0, 110.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<false>{},
                      1,
                      OpenMethod{},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      1.0,
                      single_take_profit(ValueMethod{120.0}, true)),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_EQ(timeline.closed_trades(last_timeline_index(timeline)).size(), 1);
  EXPECT_EQ(latest_closed_trade(timeline).exit_type(),
            TradeEvent::Type::take_profit);
  ASSERT_EQ(latest_closed_trade(timeline).take_profit_levels().size(), 1);
  EXPECT_DOUBLE_EQ(
   latest_closed_trade(timeline).take_profit_levels().front().price(), 120.0);
  EXPECT_DOUBLE_EQ(latest_closed_trade(timeline).exit_price(), 120.0);
}

TEST(BacktestRunnerTest, SignalExitReducesRemainingPositionOnlyOnce)
{
  const auto asset = make_three_bar_asset(100.0,
                                          100.0,
                                          100.0,
                                          100.0,
                                          110.0,
                                          110.0,
                                          110.0,
                                          110.0,
                                          120.0,
                                          120.0,
                                          120.0,
                                          120.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{4.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  make_position_rule(BooleanMethod<true>{},
                                     BooleanMethod<true>{},
                                     BooleanMethod<false>{},
                                     1,
                                     OpenMethod{},
                                     false,
                                     false,
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     0,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     0.5),
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(timeline.open_position(1).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 2.0);
  EXPECT_TRUE(timeline.closed_trades(1).empty());
  ASSERT_EQ(timeline.trade_events(1).size(), 1);
  EXPECT_EQ(timeline.trade_events(1).front().type(),
            TradeEvent::Type::exit_signal);
  EXPECT_DOUBLE_EQ(timeline.capital(1), 1020.0);
  EXPECT_EQ(timeline.trade_count(1), 0);

  runner.run(series_results, timeline);

  ASSERT_TRUE(timeline.open_position(2).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(2)->position_size(), 2.0);
  EXPECT_TRUE(timeline.trade_events(2).empty());
  EXPECT_DOUBLE_EQ(timeline.capital(2), 1020.0);
}

TEST(BacktestRunnerTest, OrderedSignalExitsExecuteOnceAndResetAfterFullClosure)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0, 3.0, 4.0}},
                                        {"Open", {100.0, 110.0, 120.0, 130.0}},
                                        {"High", {100.0, 110.0, 120.0, 130.0}},
                                        {"Low", {100.0, 110.0, 120.0, 130.0}},
                                        {"Close", {100.0, 110.0, 120.0, 130.0}},
                                        {"Volume", {0.0, 0.0, 0.0, 0.0}}}};
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto signal_exits =
   std::vector<BacktestRunner::PositionRule::SignalExitRule>{};
  signal_exits.emplace_back(
   false, BooleanMethod<true>{}, SignalTiming::NextOpen, 1.0);
  signal_exits.emplace_back(
   true, BooleanMethod<true>{}, SignalTiming::NextOpen, 0.5);
  signal_exits.emplace_back(
   true, BooleanMethod<true>{}, SignalTiming::NextOpen, 1.0);

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
                                std::move(signal_exits),
                                BooleanMethod<false>{},
                                1,
                                ValueMethod{10.0},
                                single_stop_loss(ValueMethod{90.0}, false)},
   BacktestRunner::PositionRule{},
   5000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  EXPECT_FALSE(timeline.open_position(1));
  ASSERT_EQ(timeline.trade_events(1).size(), 2);
  ASSERT_EQ(timeline.closed_trades(1).size(), 1);
  ASSERT_EQ(timeline.closed_trades(1).front().signal_exit_states().size(), 3);
  EXPECT_DOUBLE_EQ(timeline.closed_trades(1).front().risk_distance(), 10.0);
  EXPECT_DOUBLE_EQ(timeline.closed_trades(1).front().risk_reference_price(),
                   100.0);
  EXPECT_DOUBLE_EQ(timeline.closed_trades(1).front().risk_boundary_price(),
                   90.0);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).front().risk_distance(), 10.0);
  EXPECT_TRUE(
   timeline.closed_trades(1).front().signal_exit_states()[1].consumed());
  EXPECT_TRUE(
   timeline.closed_trades(1).front().signal_exit_states()[2].consumed());

  runner.run(series_results, timeline);

  ASSERT_TRUE(timeline.open_position(2));
  ASSERT_EQ(timeline.open_position(2)->signal_exit_states().size(), 3);
  EXPECT_FALSE(timeline.open_position(2)->signal_exit_states()[1].consumed());
  EXPECT_FALSE(timeline.open_position(2)->signal_exit_states()[2].consumed());
}

TEST(BacktestRunnerTest, AfterPreviousSignalExitsAdvanceAtTheSameCurrentClose)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 110.0, 110.0, 110.0, 110.0);
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto signal_exits =
   std::vector<BacktestRunner::PositionRule::SignalExitRule>{};
  signal_exits.emplace_back(
   true, BooleanMethod<true>{}, SignalTiming::CurrentClose, 0.5);
  signal_exits.emplace_back(
   true, BooleanMethod<true>{}, SignalTiming::CurrentClose, 1.0);
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
                                std::move(signal_exits),
                                BooleanMethod<false>{},
                                1,
                                ValueMethod{10.0},
                                {},
                                SignalTiming::CurrentClose,
                                SignalTiming::NextOpen,
                                StopTargetReferencePrice::AveragePrice,
                                StopTargetReferencePrice::AveragePrice,
                                {},
                                ExitActivation::AfterPrevious},
   BacktestRunner::PositionRule{},
   5000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  EXPECT_FALSE(timeline.open_position(1));
  ASSERT_EQ(timeline.trade_events(1).size(), 2);
  EXPECT_EQ(timeline.trade_events(1)[0].type(), TradeEvent::Type::exit_signal);
  EXPECT_EQ(timeline.trade_events(1)[1].type(), TradeEvent::Type::exit_signal);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1)[0].price(), 110.0);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1)[1].price(), 110.0);
}

TEST(BacktestRunnerTest, PyramidingPreservesConsumedSignalExitStates)
{
  const auto asset = make_three_bar_asset(100.0,
                                          100.0,
                                          100.0,
                                          100.0,
                                          110.0,
                                          110.0,
                                          110.0,
                                          110.0,
                                          120.0,
                                          120.0,
                                          120.0,
                                          120.0);
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto signal_exits =
   std::vector<BacktestRunner::PositionRule::SignalExitRule>{};
  signal_exits.emplace_back(
   true, BooleanMethod<true>{}, SignalTiming::NextOpen, 0.5);
  signal_exits.emplace_back(
   true, BooleanMethod<false>{}, SignalTiming::NextOpen, 1.0);

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
                                std::move(signal_exits),
                                BooleanMethod<true>{},
                                2,
                                ValueMethod{10.0},
                                single_stop_loss(ValueMethod{90.0}, false)},
   BacktestRunner::PositionRule{},
   5000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 12.0);
  EXPECT_TRUE(timeline.open_position(1)->signal_exit_states()[0].consumed());

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(2));
  EXPECT_DOUBLE_EQ(timeline.open_position(2)->position_size(), 12.0);
  EXPECT_TRUE(timeline.open_position(2)->signal_exit_states()[0].consumed());
  EXPECT_FALSE(timeline.open_position(2)->signal_exit_states()[1].consumed());
}

TEST(BacktestRunnerTest, StopLossAndTakeProfitHavePriorityOverSignalExits)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0, 3.0, 4.0}},
                                        {"Open", {100.0, 95.0, 110.0, 120.0}},
                                        {"High", {100.0, 121.0, 121.0, 120.0}},
                                        {"Low", {100.0, 89.0, 100.0, 120.0}},
                                        {"Close", {100.0, 95.0, 110.0, 120.0}},
                                        {"Volume", {0.0, 0.0, 0.0, 0.0}}}};
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto signal_exits =
   std::vector<BacktestRunner::PositionRule::SignalExitRule>{};
  signal_exits.emplace_back(
   true, BooleanMethod<true>{}, SignalTiming::NextOpen, 0.5);

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
                   std::move(signal_exits),
                   BooleanMethod<false>{},
                   1,
                   ValueMethod{10.0},
                   single_stop_loss(ValueMethod{90.0}, true, false, 0.25),
                   SignalTiming::NextOpen,
                   SignalTiming::NextOpen,
                   StopTargetReferencePrice::AveragePrice,
                   StopTargetReferencePrice::AveragePrice,
                   single_take_profit(ValueMethod{120.0}, true, 0.5)},
                  BacktestRunner::PositionRule{},
                  5000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  ASSERT_EQ(timeline.trade_events(1).size(), 3);
  EXPECT_EQ(timeline.trade_events(1)[1].type(), TradeEvent::Type::stop_loss);
  EXPECT_EQ(timeline.trade_events(1)[2].type(), TradeEvent::Type::take_profit);
  EXPECT_FALSE(timeline.open_position(1)->signal_exit_states()[0].consumed());
  EXPECT_TRUE(timeline.open_position(1)->take_profit_levels()[0].consumed());

  runner.run(series_results, timeline);
  ASSERT_EQ(timeline.trade_events(2).size(), 1);
  EXPECT_EQ(timeline.trade_events(2).front().type(),
            TradeEvent::Type::exit_signal);
  EXPECT_TRUE(timeline.open_position(2)->signal_exit_states()[0].consumed());

  runner.run(series_results, timeline);
  EXPECT_TRUE(timeline.trade_events(3).empty());
  EXPECT_TRUE(timeline.open_position(3)->signal_exit_states()[0].consumed());
}

auto run_signal_reduction(double reduce,
                          double position_quantity = 10.0,
                          double minimum_quantity = 1.0,
                          double quantity_step = 1.0) -> BacktestTimeline
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 110.0, 110.0, 110.0, 110.0);
  const auto market = Market{"Test", minimum_quantity, quantity_step};
  const auto broker = Broker{"Test"};
  const auto profile = Profile{
   "Test", PositionSizingNode{FixedQuantityPositionSizing{position_quantity}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  make_position_rule(BooleanMethod<true>{},
                                     BooleanMethod<true>{},
                                     BooleanMethod<false>{},
                                     1,
                                     OpenMethod{},
                                     false,
                                     false,
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     0,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     reduce),
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  return timeline;
}

TEST(BacktestRunnerTest, NearestReductionRoundsHalfStepUp)
{
  const auto timeline = run_signal_reduction(0.25);

  ASSERT_TRUE(timeline.open_position(1).has_value());
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).front().position_size(), 3.0);
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 7.0);
}

TEST(BacktestRunnerTest, NearestReductionCanRoundDown)
{
  const auto timeline = run_signal_reduction(0.21);

  ASSERT_TRUE(timeline.open_position(1).has_value());
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).front().position_size(), 2.0);
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 8.0);
}

TEST(BacktestRunnerTest, ReductionClosesPositionInsteadOfLeavingDust)
{
  const auto timeline = run_signal_reduction(0.9, 10.0, 2.0, 1.0);

  EXPECT_FALSE(timeline.open_position(1).has_value());
  ASSERT_EQ(timeline.closed_trades(1).size(), 1);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).front().position_size(), 10.0);
}

TEST(BacktestRunnerTest, StopLossAndTakeProfitSupportIndependentReductions)
{
  const auto asset = make_three_bar_asset(100.0,
                                          100.0,
                                          100.0,
                                          100.0,
                                          95.0,
                                          96.0,
                                          89.0,
                                          95.0,
                                          120.0,
                                          121.0,
                                          110.0,
                                          120.0);
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      BooleanMethod<false>{},
                      BooleanMethod<false>{},
                      1,
                      ValueMethod{90.0},
                      true,
                      false,
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      1.0,
                      0.26,
                      single_take_profit(ValueMethod{120.0}, true, 0.5)),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(1).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 6.0);
  EXPECT_EQ(timeline.trade_events(1).front().type(),
            TradeEvent::Type::stop_loss);
  EXPECT_TRUE(timeline.trade_events(1).front().is_scale_out());

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(2).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(2)->position_size(), 3.0);
  EXPECT_EQ(timeline.trade_events(2).front().type(),
            TradeEvent::Type::take_profit);
  EXPECT_TRUE(timeline.trade_events(2).front().is_scale_out());
  EXPECT_TRUE(timeline.closed_trades(2).empty());
}

TEST(BacktestRunnerTest,
     OrderedTakeProfitsTriggerOnceAndReduceRemainingPosition)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0, 3.0, 4.0}},
                                        {"Open", {100.0, 100.0, 100.0, 100.0}},
                                        {"High", {100.0, 131.0, 121.0, 131.0}},
                                        {"Low", {100.0, 100.0, 100.0, 100.0}},
                                        {"Close", {100.0, 120.0, 120.0, 130.0}},
                                        {"Volume", {0.0, 0.0, 0.0, 0.0}}}};
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto take_profits =
   std::vector<BacktestRunner::PositionRule::TakeProfitRule>{};
  take_profits.emplace_back(ValueMethod{110.0}, false, 1.0);
  take_profits.emplace_back(ValueMethod{120.0}, true, 0.5);
  take_profits.emplace_back(ValueMethod{130.0}, true, 0.5);

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  make_position_rule(BooleanMethod<true>{},
                                     BooleanMethod<false>{},
                                     BooleanMethod<false>{},
                                     1,
                                     ValueMethod{90.0},
                                     false,
                                     false,
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     1.0,
                                     1.0,
                                     std::move(take_profits)),
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 2.0);
  ASSERT_EQ(timeline.open_position(1)->take_profit_levels().size(), 3);
  EXPECT_FALSE(timeline.open_position(1)->take_profit_levels()[0].enabled());
  EXPECT_TRUE(timeline.open_position(1)->take_profit_levels()[1].consumed());
  EXPECT_TRUE(timeline.open_position(1)->take_profit_levels()[2].consumed());
  ASSERT_EQ(timeline.trade_events(1).size(), 2);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).front().price(), 120.0);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).back().price(), 130.0);

  runner.run(series_results, timeline);
  EXPECT_TRUE(timeline.trade_events(2).empty());
  EXPECT_DOUBLE_EQ(timeline.open_position(2)->position_size(), 2.0);

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(3));
  EXPECT_DOUBLE_EQ(timeline.open_position(3)->position_size(), 2.0);
  EXPECT_TRUE(timeline.open_position(3)->take_profit_levels()[2].consumed());
  EXPECT_TRUE(timeline.trade_events(3).empty());
}

TEST(BacktestRunnerTest,
     PyramidingRecalculatesTargetsWithoutRearmingConsumedLevels)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0, 3.0}},
                                        {"Open", {100.0, 100.0, 120.0}},
                                        {"High", {100.0, 111.0, 120.0}},
                                        {"Low", {100.0, 100.0, 120.0}},
                                        {"Close", {100.0, 110.0, 120.0}},
                                        {"Volume", {0.0, 0.0, 0.0}}}};
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto take_profits =
   std::vector<BacktestRunner::PositionRule::TakeProfitRule>{};
  take_profits.emplace_back(TpAmountMethod{10.0}, true, 0.5);
  take_profits.emplace_back(TpAmountMethod{30.0}, true, 0.5);

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  make_position_rule(BooleanMethod<true>{},
                                     BooleanMethod<false>{},
                                     BooleanMethod<true>{},
                                     2,
                                     ValueMethod{90.0},
                                     false,
                                     false,
                                     0,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     0,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     1.0,
                                     1.0,
                                     std::move(take_profits)),
                  BacktestRunner::PositionRule{},
                  5000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 12.0);
  EXPECT_TRUE(timeline.open_position(1)->take_profit_levels()[0].consumed());
  EXPECT_NE(timeline.open_position(1)->take_profit_levels()[0].price(), 110.0);

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(2));
  EXPECT_DOUBLE_EQ(timeline.open_position(2)->position_size(), 12.0);
  EXPECT_TRUE(timeline.open_position(2)->take_profit_levels()[0].consumed());
  EXPECT_NE(timeline.open_position(2)->take_profit_levels()[0].price(), 110.0);
  EXPECT_FALSE(timeline.open_position(2)->take_profit_levels()[1].consumed());
}

TEST(BacktestRunnerTest,
     AfterPreviousActivatesMarketableTakeProfitImmediatelyAndSkipsDisabledRules)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 100.0, 130.0, 100.0, 125.0);
  const auto market = Market{"Test", 1.0, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{8.0}}};
  auto take_profits =
   std::vector<BacktestRunner::PositionRule::TakeProfitRule>{};
  take_profits.emplace_back(ValueMethod{200.0}, false, 1.0);
  take_profits.emplace_back(ValueMethod{120.0}, true, 0.5);
  take_profits.emplace_back(ValueMethod{110.0}, true, 0.5);
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
                                {},
                                BooleanMethod<false>{},
                                1,
                                ValueMethod{10.0},
                                {},
                                SignalTiming::CurrentClose,
                                SignalTiming::NextOpen,
                                StopTargetReferencePrice::AveragePrice,
                                StopTargetReferencePrice::AveragePrice,
                                std::move(take_profits),
                                ExitActivation::Simultaneous,
                                ExitActivation::Simultaneous,
                                ExitActivation::AfterPrevious},
   BacktestRunner::PositionRule{},
   5000.0,
   0,
   false,
   NAN,
   IntrabarPath::LowFirst};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 2.0);
  ASSERT_EQ(timeline.open_position(1)->take_profit_levels().size(), 3);
  EXPECT_FALSE(timeline.open_position(1)->take_profit_levels()[0].consumed());
  EXPECT_TRUE(timeline.open_position(1)->take_profit_levels()[1].consumed());
  EXPECT_TRUE(timeline.open_position(1)->take_profit_levels()[2].consumed());
  ASSERT_EQ(timeline.trade_events(1).size(), 2);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1)[0].price(), 120.0);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1)[1].price(), 120.0);
}

TEST(BacktestRunnerTest, ReductionIsRaisedToMarketMinimum)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 110.0, 110.0, 110.0, 110.0);
  const auto market = Market{"Test", 1.5, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{4.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  make_position_rule(BooleanMethod<true>{},
                                     BooleanMethod<true>{},
                                     BooleanMethod<false>{},
                                     1,
                                     OpenMethod{},
                                     false,
                                     false,
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     1,
                                     OpenMethod{},
                                     StopTargetReferencePrice::AveragePrice,
                                     StopTargetReferencePrice::AveragePrice,
                                     0.1),
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(timeline.open_position(1).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 2.0);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).front().position_size(), 2.0);
}

TEST(BacktestRunnerTest, PartialExitKeepsPyramidingLayersUsed)
{
  const auto asset = Asset{"Test",
                           AssetHistory{{"Datetime", {1.0, 2.0, 3.0, 4.0}},
                                        {"Open", {100.0, 110.0, 120.0, 130.0}},
                                        {"High", {100.0, 110.0, 120.0, 130.0}},
                                        {"Low", {100.0, 110.0, 120.0, 130.0}},
                                        {"Close", {100.0, 110.0, 120.0, 130.0}},
                                        {"Volume", {0.0, 0.0, 0.0, 0.0}}}};
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizingNode{FixedQuantityPositionSizing{4.0}}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   make_position_rule(BooleanMethod<true>{},
                      EqualMethod{CloseMethod{}, ValueMethod{120.0}},
                      BooleanMethod<true>{},
                      2,
                      OpenMethod{},
                      false,
                      false,
                      1,
                      OpenMethod{},
                      0,
                      OpenMethod{},
                      1,
                      OpenMethod{},
                      StopTargetReferencePrice::AveragePrice,
                      StopTargetReferencePrice::AveragePrice,
                      0.5),
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(1).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 8.0);

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(2).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(2)->position_size(), 4.0);

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(3).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(3)->position_size(), 4.0);
  EXPECT_TRUE(timeline.trade_events(3).empty());
}
