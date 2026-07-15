#include <gtest/gtest.h>

#include <algorithm>
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

auto run_single_entry(
 PositionSizing position_sizing,
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

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
                   BooleanMethod<false>{},
                   BooleanMethod<false>{},
                   1,
                   ValueMethod{std::numeric_limits<double>::quiet_NaN()},
                   false,
                   false},
                  BacktestRunner::PositionRule{},
                  initial_capital,
                  0,
                  false,
                  peak_equity};

  runner.run(series_results, timeline);

  return timeline;
}

auto run_single_close_price_entry(
 PositionSizing position_sizing,
 double open_price = 100.0,
 double close_price = 125.0,
 double stop_price = std::numeric_limits<double>::quiet_NaN(),
 Broker broker = Broker{"Test"}) -> BacktestTimeline
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
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               BooleanMethod<false>{},
                                               BooleanMethod<false>{},
                                               1,
                                               ValueMethod{stop_price},
                                               false,
                                               false,
                                               0,
                                               CloseMethod{}},
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

auto latest_closed_trade(const BacktestTimeline& timeline) -> const ClosedTrade&
{
  const auto timeline_i = last_timeline_index(timeline);
  return timeline.closed_trades(timeline_i).back();
}

auto single_take_profit(AnySeriesMethod target_price,
                        bool enabled,
                        double reduce = 1.0)
 -> std::vector<BacktestRunner::PositionRule::TakeProfitRule>
{
  auto take_profits =
   std::vector<BacktestRunner::PositionRule::TakeProfitRule>{};
  take_profits.emplace_back(std::move(target_price), enabled, reduce);
  return take_profits;
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::RiskDistance, 0.10}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto entry_signal =
   LogicalOrMethod{EqualMethod{CloseMethod{}, ValueMethod{50.0}},
                   EqualMethod{CloseMethod{}, ValueMethod{110.0}}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{entry_signal,
                                               BooleanMethod<true>{},
                                               BooleanMethod<false>{},
                                               1,
                                               ValueMethod{90.0},
                                               false,
                                               false},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(
   timeline.open_position(last_timeline_index(timeline))->position_size(),
   10.0);

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 3);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);
  EXPECT_DOUBLE_EQ(timeline.equity(last_timeline_index(timeline)), 1100.0);

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 4);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(
   timeline.open_position(last_timeline_index(timeline))->position_size(), 5.5);
}

TEST(BacktestRunnerTest,
     DefaultRejectRecordsOversizedEntryWithoutOpeningPosition)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 20.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               BooleanMethod<false>{},
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               0,
                                               OpenMethod{},
                                               0,
                                               OpenMethod{}},
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
}

TEST(BacktestRunnerTest, CapToAvailableCashOpensLargestAffordableOrder)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test",
           PositionSizing{PositionSizing::Mode::FixedQuantity, 20.0},
           DrawdownAdjustment{},
           InsufficientCashPolicy::CapToAvailableCash};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               BooleanMethod<false>{},
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               0,
                                               OpenMethod{}},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 1);
  ASSERT_TRUE(timeline.open_position(0).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(0)->position_size(), 10.0);
  ASSERT_EQ(timeline.trade_events(0).size(), 1);
  EXPECT_TRUE(timeline.trade_events(0).front().is_entry());
}

TEST(BacktestRunnerTest, CapToAvailableCashSkipsWhenBelowMarketMinimum)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 11.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test",
           PositionSizing{PositionSizing::Mode::FixedQuantity, 20.0},
           DrawdownAdjustment{},
           InsufficientCashPolicy::CapToAvailableCash};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               BooleanMethod<false>{},
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               0,
                                               OpenMethod{}},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 1);
  EXPECT_FALSE(timeline.open_position(0).has_value());
  EXPECT_TRUE(timeline.trade_events(0).empty());
  EXPECT_EQ(timeline.trade_count(0), 0);
}

TEST(BacktestRunnerTest, RejectedPyramidingDoesNotChangePosition)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 6.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               BacktestRunner::PositionRule{
                                BooleanMethod<true>{},
                                BooleanMethod<false>{},
                                BooleanMethod<true>{},
                                2,
                                OpenMethod{},
                                false,
                                false,
                               },
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
  series_methods.set("equity", EquityMethod{});

  const auto entry_signal = EqualMethod{EquityMethod{}, ValueMethod{1000.0}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  std::move(series_methods),
                  BacktestRunner::PositionRule{entry_signal,
                                               BooleanMethod<false>{},
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               0,
                                               OpenMethod{}},
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
  series_methods.set("equity_percent", EquityPercentMethod{});

  const auto entry_signal =
   EqualMethod{EquityPercentMethod{}, ValueMethod{100.0}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  std::move(series_methods),
                  BacktestRunner::PositionRule{entry_signal,
                                               BooleanMethod<false>{},
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               0,
                                               OpenMethod{}},
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
  series_methods.set("drawdown", DrawdownMethod{});

  const auto entry_signal = EqualMethod{CloseMethod{}, ValueMethod{100.0}};
  const auto exit_signal = EqualMethod{CloseMethod{}, ValueMethod{90.0}};
  const auto short_entry_signal =
   EqualMethod{DrawdownMethod{}, ValueMethod{1.0}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  std::move(series_methods),
                  BacktestRunner::PositionRule{entry_signal,
                                               exit_signal,
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               0,
                                               OpenMethod{}},
                  BacktestRunner::PositionRule{short_entry_signal,
                                               BooleanMethod<false>{},
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               0,
                                               OpenMethod{}},
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
  const auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
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
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 12.5},
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
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedNotional, 250.0});

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
   PositionSizing{PositionSizing::Mode::FixedNotional, 250.0},
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

TEST(BacktestRunnerTest, EquityPercentSizingConvertsByCurrentEquity)
{
  const auto timeline =
   run_single_entry(PositionSizing{PositionSizing::Mode::EquityPercent, 0.25});

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.5);
}

TEST(BacktestRunnerTest, EquityPercentSizingConvertsBySelectedEntryPrice)
{
  const auto timeline = run_single_close_price_entry(
   PositionSizing{PositionSizing::Mode::EquityPercent, 0.25});

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 125.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
}

TEST(BacktestRunnerTest, RiskDistanceSizingUsesSelectedEntryPrice)
{
  const auto timeline = run_single_close_price_entry(
   PositionSizing{PositionSizing::Mode::RiskDistance, 0.10},
   100.0,
   125.0,
   75.0);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 125.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 75.0);
  EXPECT_TRUE(std::isnan(latest_position(timeline).stop_loss_price()));
}

TEST(BacktestRunnerTest, DisabledTakeProfitKeepsTargetReferencePrice)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
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
                                single_take_profit(ValueMethod{120.0}, false)},
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 90.0);
  ASSERT_EQ(latest_position(timeline).take_profit_levels().size(), 1);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 120.0);
  EXPECT_TRUE(std::isnan(latest_position(timeline).stop_loss_price()));
  EXPECT_FALSE(
   latest_position(timeline).take_profit_levels().front().enabled());
}

TEST(BacktestRunnerTest, StopTargetAmountMethodsUseEntryDirection)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
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
                                single_take_profit(TpAmountMethod{20.0}, true)},
   1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 100.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), -1.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 110.0);
  ASSERT_EQ(latest_position(timeline).take_profit_levels().size(), 1);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 80.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_loss_price(), 110.0);
  EXPECT_TRUE(latest_position(timeline).take_profit_levels().front().active());
}

TEST(BacktestRunnerTest, ScopedStopTargetAmountMethodsEvaluateDirectly)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto snapshot = asset.get_snapshot(0);
  const auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
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
  const auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
  auto series_results = SeriesEvaluationResults{};
  auto default_context = DefaultMethodContext{series_methods, series_results};
  const auto account_state = BacktestAccountState{1000.0, 0.0, 1000.0, 1000.0};
  auto context =
   BacktestMethodContext{default_context, series_methods, account_state};
  const auto scoped_context =
   context.with_position_prices(90.0, 120.0, 105.0, 110.0, -1.0);

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
}

TEST(BacktestRunnerTest, StopTargetPercentageMethodsUseEntryPrice)
{
  const auto asset = make_single_bar_asset(200.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(TpPercentMethod{20.0}, true)},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 180.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 240.0);
}

TEST(BacktestRunnerTest, StopTargetPercentageMethodsUseFeeAdjustedAveragePrice)
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(TpRMultipleMethod{2.0}, false)},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 99.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 132.0);
}

TEST(BacktestRunnerTest, ScopedStopTargetPercentMethodsEvaluateDirectly)
{
  const auto asset = make_single_bar_asset(200.0);
  const auto snapshot = asset.get_snapshot(0);
  const auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(TpRMultipleMethod{2.0}, true)},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).entry_price(), 100.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 60.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 180.0);
}

TEST(BacktestRunnerTest, PositionContextMethodsUseNormalLongEntryContext)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(PositionDirectionMethod{}, false)},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 100.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 1.0);
}

TEST(BacktestRunnerTest, PositionContextMethodsUseNormalShortEntryContext)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(PositionDirectionMethod{}, false)},
                  1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 100.0);
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(LatestEntryPriceMethod{}, false)},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 110.0);
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(StopTargetRefPriceMethod{}, false)},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 100.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 120.0);
}

TEST(BacktestRunnerTest, CustomStopTargetFormulasUseReferenceAndDirection)
{
  const auto asset = make_single_bar_asset(100.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto stop_method =
   SubtractMethod{StopTargetRefPriceMethod{},
                  MultiplyMethod{PositionDirectionMethod{}, ValueMethod{10.0}}};
  const auto target_method =
   AddMethod{StopTargetRefPriceMethod{},
             MultiplyMethod{PositionDirectionMethod{}, ValueMethod{20.0}}};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
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
                                single_take_profit(target_method, false)},
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 90.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 120.0);
}

TEST(BacktestRunnerTest, ScopedAtrStopAndRMultipleTargetEvaluateDirectly)
{
  const auto asset =
   make_single_bar_asset_with_range(100.0, 110.0, 90.0, 100.0);
  const auto snapshot = asset.get_snapshot(0);
  const auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
  auto series_results = SeriesEvaluationResults{};
  auto default_context = DefaultMethodContext{series_methods, series_results};
  const auto account_state = BacktestAccountState{1000.0, 0.0, 1000.0, 1000.0};
  auto context =
   BacktestMethodContext{default_context, series_methods, account_state};
  const auto stop_context = context.with_position_reference(100.0, 1.0);
  const auto stop_price =
   evaluate_series_method(SlAtrMethod{1.0, 2.0}, snapshot, stop_context);
  const auto target_context = stop_context.with_position_stop_price(stop_price);

  EXPECT_DOUBLE_EQ(stop_price, 60.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(TpRMultipleMethod{2.0}, snapshot, target_context),
   180.0);
}

TEST(BacktestRunnerTest, DisabledDrawdownAdjustmentLeavesSizingUnchanged)
{
  const auto timeline =
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 100.0},
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
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 100.0},
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
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 100.0},
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
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 100.0},
                    100.0,
                    DrawdownAdjustment{true, 0.10, 0.20},
                    8000.0,
                    10000.0);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 60.0);
}

TEST(BacktestRunnerTest, DrawdownAdjustmentClampsSizeAtZero)
{
  const auto timeline =
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 100.0},
                    100.0,
                    DrawdownAdjustment{true, 0.10, 0.20},
                    400.0,
                    1000.0);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 0.0);
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
  series_methods.set("close", CloseMethod{});

  const auto entry_signal =
   EqualMethod{SeriesMethod{"close"}, ValueMethod{20.0}};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               std::move(series_methods),
                               BacktestRunner::PositionRule{
                                entry_signal,
                                BooleanMethod<false>{},
                                BooleanMethod<false>{},
                                1,
                                OpenMethod{},
                                false,
                                false,
                               },
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
  series_methods.set("close", CloseMethod{});

  const auto entry_signal =
   EqualMethod{SeriesMethod{"close"}, ValueMethod{125.0}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  std::move(series_methods),
                  BacktestRunner::PositionRule{entry_signal,
                                               BooleanMethod<false>{},
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               0,
                                               OpenMethod{}},
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedNotional, 250.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               BooleanMethod<false>{},
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               0,
                                               CloseMethod{}},
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto entry_signal = EqualMethod{CloseMethod{}, ValueMethod{75.0}};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               BacktestRunner::PositionRule{
                                entry_signal,
                                BooleanMethod<false>{},
                                BooleanMethod<false>{},
                                1,
                                OpenMethod{},
                                false,
                                false,
                               },
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
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
                                               BooleanMethod<false>{},
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               0,
                                               CloseMethod{}},
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto exit_signal = EqualMethod{CloseMethod{}, ValueMethod{120.0}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               exit_signal,
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               1,
                                               OpenMethod{},
                                               0,
                                               CloseMethod{}},
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  const auto pyramiding_signal = EqualMethod{CloseMethod{}, ValueMethod{120.0}};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
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
                                               CloseMethod{}},
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(TpRMultipleMethod{2.0}, false)},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 99.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 132.0);
}

TEST(BacktestRunnerTest, PyramidingUsesPostScaleInFeeAdjustedAverageReference)
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(TpRMultipleMethod{2.0}, false)},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 120.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 108.0);
  EXPECT_DOUBLE_EQ(
   latest_position(timeline).take_profit_levels().front().price(), 144.0);
}

TEST(BacktestRunnerTest,
     UnfavorableLongPyramidingUsesPostActionAverageReferenceByDefault)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 80.0, 80.0, 80.0, 80.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(TpRMultipleMethod{2.0}, false)},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 90.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 81.0);
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(TpRMultipleMethod{2.0}, false)},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), -2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 90.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 99.0);
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(TpRMultipleMethod{2.0}, false)},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 108.0);
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(TpRMultipleMethod{2.0}, false)},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), -2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 121.0);
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(TpRMultipleMethod{2.0}, false)},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(
   timeline.open_position(last_timeline_index(timeline)).has_value());
  EXPECT_DOUBLE_EQ(latest_position(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).average_price(), 110.0);
  EXPECT_DOUBLE_EQ(latest_position(timeline).stop_price(), 90.0);
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               BacktestRunner::PositionRule{
                                BooleanMethod<true>{},
                                BooleanMethod<true>{},
                                BooleanMethod<false>{},
                                1,
                                OpenMethod{},
                                false,
                                false,
                               },
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               BacktestRunner::PositionRule{
                                BooleanMethod<true>{},
                                BooleanMethod<true>{},
                                BooleanMethod<true>{},
                                2,
                                OpenMethod{},
                                false,
                                false,
                               },
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               BacktestRunner::PositionRule{
                                BooleanMethod<true>{},
                                BooleanMethod<true>{},
                                BooleanMethod<false>{},
                                1,
                                OpenMethod{},
                                false,
                                false,
                               },
                               BacktestRunner::PositionRule{
                                BooleanMethod<true>{},
                                BooleanMethod<false>{},
                                BooleanMethod<false>{},
                                1,
                                OpenMethod{},
                                false,
                                false,
                               },
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               BacktestRunner::PositionRule{
                                BooleanMethod<true>{},
                                BooleanMethod<false>{},
                                BooleanMethod<false>{},
                                1,
                                ValueMethod{90.0},
                                true,
                                false,
                               },
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_EQ(timeline.closed_trades(last_timeline_index(timeline)).size(), 1);
  EXPECT_EQ(latest_closed_trade(timeline).exit_type(),
            TradeEvent::Type::stop_loss);
  EXPECT_DOUBLE_EQ(latest_closed_trade(timeline).stop_price(), 90.0);
  EXPECT_DOUBLE_EQ(latest_closed_trade(timeline).stop_loss_price(), 90.0);
  EXPECT_DOUBLE_EQ(latest_closed_trade(timeline).exit_price(), 90.0);
}

TEST(BacktestRunnerTest, TrailingStopMutationUsesTradePositionState)
{
  const auto asset =
   make_two_bar_asset(100.0, 115.0, 100.0, 115.0, 110.0, 112.0, 104.0, 110.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               BacktestRunner::PositionRule{
                                BooleanMethod<true>{},
                                BooleanMethod<false>{},
                                BooleanMethod<false>{},
                                1,
                                ValueMethod{90.0},
                                true,
                                true,
                               },
                               BacktestRunner::PositionRule{},
                               1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_EQ(timeline.closed_trades(last_timeline_index(timeline)).size(), 1);
  EXPECT_EQ(latest_closed_trade(timeline).exit_type(),
            TradeEvent::Type::stop_loss);
  EXPECT_DOUBLE_EQ(latest_closed_trade(timeline).stop_price(), 90.0);
  EXPECT_DOUBLE_EQ(latest_closed_trade(timeline).stop_loss_price(), 105.0);
  EXPECT_DOUBLE_EQ(latest_closed_trade(timeline).exit_price(), 105.0);
}

TEST(BacktestRunnerTest, TakeProfitExitIsDecidedByRunner)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 110.0, 121.0, 109.0, 110.0);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 1.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
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
                                single_take_profit(ValueMethod{120.0}, true)},
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 4.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
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
                                0.5},
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
   "Test",
   PositionSizing{PositionSizing::Mode::FixedQuantity, position_quantity}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
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
                                reduce},
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 8.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {},
                  BacktestRunner::PositionRule{
                   BooleanMethod<true>{},
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
                   single_take_profit(ValueMethod{120.0}, true, 0.5)},
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 8.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto take_profits =
   std::vector<BacktestRunner::PositionRule::TakeProfitRule>{};
  take_profits.emplace_back(ValueMethod{110.0}, false, 1.0);
  take_profits.emplace_back(ValueMethod{120.0}, true, 0.5);
  take_profits.emplace_back(ValueMethod{130.0}, true, 0.5);

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
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
                                std::move(take_profits)},
   BacktestRunner::PositionRule{},
   1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 4.0);
  ASSERT_EQ(timeline.open_position(1)->take_profit_levels().size(), 3);
  EXPECT_FALSE(timeline.open_position(1)->take_profit_levels()[0].enabled());
  EXPECT_TRUE(timeline.open_position(1)->take_profit_levels()[1].consumed());
  EXPECT_FALSE(timeline.open_position(1)->take_profit_levels()[2].consumed());
  ASSERT_EQ(timeline.trade_events(1).size(), 1);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1).front().price(), 120.0);

  runner.run(series_results, timeline);
  EXPECT_TRUE(timeline.trade_events(2).empty());
  EXPECT_DOUBLE_EQ(timeline.open_position(2)->position_size(), 4.0);

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(3));
  EXPECT_DOUBLE_EQ(timeline.open_position(3)->position_size(), 2.0);
  EXPECT_TRUE(timeline.open_position(3)->take_profit_levels()[2].consumed());
  ASSERT_EQ(timeline.trade_events(3).size(), 1);
  EXPECT_EQ(timeline.trade_events(3).front().type(),
            TradeEvent::Type::take_profit);
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 8.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto take_profits =
   std::vector<BacktestRunner::PositionRule::TakeProfitRule>{};
  take_profits.emplace_back(TpAmountMethod{10.0}, true, 0.5);
  take_profits.emplace_back(TpAmountMethod{30.0}, true, 0.5);

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
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
                                std::move(take_profits)},
   BacktestRunner::PositionRule{},
   5000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 4.0);
  EXPECT_TRUE(timeline.open_position(1)->take_profit_levels()[0].consumed());
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->take_profit_levels()[0].price(),
                   110.0);

  runner.run(series_results, timeline);
  ASSERT_TRUE(timeline.open_position(2));
  EXPECT_DOUBLE_EQ(timeline.open_position(2)->position_size(), 12.0);
  EXPECT_TRUE(timeline.open_position(2)->take_profit_levels()[0].consumed());
  EXPECT_NE(timeline.open_position(2)->take_profit_levels()[0].price(), 110.0);
  EXPECT_FALSE(timeline.open_position(2)->take_profit_levels()[1].consumed());
}

TEST(BacktestRunnerTest, ReductionIsRaisedToMarketMinimum)
{
  const auto asset =
   make_two_bar_asset(100.0, 100.0, 100.0, 100.0, 110.0, 110.0, 110.0, 110.0);
  const auto market = Market{"Test", 1.5, 1.0};
  const auto broker = Broker{"Test"};
  const auto profile =
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 4.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
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
                                0.1},
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
   Profile{"Test", PositionSizing{PositionSizing::Mode::FixedQuantity, 4.0}};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};

  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {},
   BacktestRunner::PositionRule{BooleanMethod<true>{},
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
                                0.5},
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
