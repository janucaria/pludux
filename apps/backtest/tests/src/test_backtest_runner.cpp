#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
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
                   false,
                   OpenMethod{},
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
                                               OpenMethod{},
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

auto latest_record(const BacktestTimeline& timeline) -> const TradeRecord&
{
  const auto timeline_i = last_timeline_index(timeline);
  return timeline.trade_records(timeline_i).back();
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
                           AssetHistory{{"Datetime", {4.0, 3.0, 2.0, 1.0}},
                                        {"Open", {110.0, 110.0, 100.0, 50.0}},
                                        {"High", {110.0, 110.0, 100.0, 50.0}},
                                        {"Low", {110.0, 110.0, 100.0, 50.0}},
                                        {"Close", {110.0, 110.0, 100.0, 50.0}},
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
                                               false,
                                               OpenMethod{},
                                               false},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(
   timeline.trade_records(last_timeline_index(timeline)).back().position_size(),
   10.0);

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 3);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);
  EXPECT_DOUBLE_EQ(timeline.equity(last_timeline_index(timeline)), 1100.0);

  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 4);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(
   timeline.trade_records(last_timeline_index(timeline)).back().position_size(),
   5.5);
}

TEST(BacktestRunnerTest, FixedQuantitySizingUsesExactQuantity)
{
  const auto timeline =
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 12.5});

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 12.5);
}

TEST(BacktestRunnerTest, FixedNotionalSizingConvertsByEntryPrice)
{
  const auto timeline =
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedNotional, 250.0});

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 2.5);
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

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).entry_price(), 125.0);
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_record(timeline).investment(), 252.5);
  EXPECT_DOUBLE_EQ(latest_record(timeline).total_entry_fees(), 2.5);
}

TEST(BacktestRunnerTest, EquityPercentSizingConvertsByCurrentEquity)
{
  const auto timeline =
   run_single_entry(PositionSizing{PositionSizing::Mode::EquityPercent, 0.25});

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 2.5);
}

TEST(BacktestRunnerTest, EquityPercentSizingConvertsBySelectedEntryPrice)
{
  const auto timeline = run_single_close_price_entry(
   PositionSizing{PositionSizing::Mode::EquityPercent, 0.25});

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).entry_price(), 125.0);
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 2.0);
}

TEST(BacktestRunnerTest, RiskDistanceSizingUsesSelectedEntryPrice)
{
  const auto timeline = run_single_close_price_entry(
   PositionSizing{PositionSizing::Mode::RiskDistance, 0.10},
   100.0,
   125.0,
   75.0);

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).entry_price(), 125.0);
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 2.0);
}

TEST(BacktestRunnerTest, DisabledDrawdownAdjustmentLeavesSizingUnchanged)
{
  const auto timeline =
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 100.0},
                    100.0,
                    DrawdownAdjustment{false, 0.10, 0.20},
                    900.0,
                    1000.0);

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 100.0);
}

TEST(BacktestRunnerTest, DrawdownAdjustmentLeavesSizeUnchangedAtZeroDrawdown)
{
  const auto timeline =
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 100.0},
                    100.0,
                    DrawdownAdjustment{true, 0.10, 0.20});

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 100.0);
}

TEST(BacktestRunnerTest, DrawdownAdjustmentReducesSizeAtTenPercentDrawdown)
{
  const auto timeline =
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 100.0},
                    100.0,
                    DrawdownAdjustment{true, 0.10, 0.20},
                    900.0,
                    1000.0);

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 80.0);
}

TEST(BacktestRunnerTest, DrawdownAdjustmentReducesSizeAtTwentyPercentDrawdown)
{
  const auto timeline =
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 100.0},
                    100.0,
                    DrawdownAdjustment{true, 0.10, 0.20},
                    800.0,
                    1000.0);

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 60.0);
}

TEST(BacktestRunnerTest, DrawdownAdjustmentClampsSizeAtZero)
{
  const auto timeline =
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 100.0},
                    100.0,
                    DrawdownAdjustment{true, 0.10, 0.20},
                    400.0,
                    1000.0);

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 0.0);
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
               AssetHistory{{"Datetime", {2.0, 1.0}},
                            {"Open", {second_open, first_open}},
                            {"High", {second_high, first_high}},
                            {"Low", {second_low, first_low}},
                            {"Close", {second_close, first_close}},
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
               AssetHistory{{"Datetime", {3.0, 2.0, 1.0}},
                            {"Open", {third_open, second_open, first_open}},
                            {"High", {third_high, second_high, first_high}},
                            {"Low", {third_low, second_low, first_low}},
                            {"Close", {third_close, second_close, first_close}},
                            {"Volume", {0.0, 0.0, 0.0}}}};
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
                                               OpenMethod{},
                                               false,
                                               0,
                                               CloseMethod{}},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).entry_price(), 125.0);
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 2.0);
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
                                               OpenMethod{},
                                               false},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);

  runner.run(series_results, timeline);

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).entry_price(), 125.0);
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
                                               OpenMethod{},
                                               false,
                                               0,
                                               CloseMethod{}},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);

  runner.run(series_results, timeline);

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).entry_price(), 75.0);
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
                                               OpenMethod{},
                                               false,
                                               1,
                                               OpenMethod{},
                                               0,
                                               CloseMethod{}},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_TRUE(latest_record(timeline).is_closed_exit_signal());
  EXPECT_DOUBLE_EQ(latest_record(timeline).exit_price(), 120.0);
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
                                               OpenMethod{},
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

  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), 2.0);
  EXPECT_DOUBLE_EQ(latest_record(timeline).investment(), 220.0);
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

  auto runner =
   BacktestRunner{asset,
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
                                               OpenMethod{},
                                               false},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 0);
  ASSERT_EQ(timeline.trade_records(last_timeline_index(timeline)).size(), 1);
  EXPECT_TRUE(latest_record(timeline).is_closed_exit_signal());
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

  auto runner =
   BacktestRunner{asset,
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
                                               OpenMethod{},
                                               false},
                  BacktestRunner::PositionRule{BooleanMethod<true>{},
                                               BooleanMethod<false>{},
                                               BooleanMethod<false>{},
                                               1,
                                               OpenMethod{},
                                               false,
                                               false,
                                               OpenMethod{},
                                               false},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  EXPECT_EQ(timeline.open_trade_count(last_timeline_index(timeline)), 1);
  ASSERT_EQ(timeline.trade_records(last_timeline_index(timeline)).size(), 2);
  EXPECT_TRUE(timeline.trade_records(last_timeline_index(timeline))
               .front()
               .is_closed_exit_signal());
  EXPECT_DOUBLE_EQ(latest_record(timeline).position_size(), -1.0);
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
                                               ValueMethod{90.0},
                                               true,
                                               false,
                                               OpenMethod{},
                                               false},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_TRUE(latest_record(timeline).is_closed_stop_loss());
  EXPECT_DOUBLE_EQ(latest_record(timeline).exit_price(), 90.0);
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
                                               ValueMethod{90.0},
                                               true,
                                               true,
                                               OpenMethod{},
                                               false},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_TRUE(latest_record(timeline).is_closed_stop_loss());
  EXPECT_DOUBLE_EQ(latest_record(timeline).trailing_stop_price(), 105.0);
  EXPECT_DOUBLE_EQ(latest_record(timeline).exit_price(), 105.0);
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
                                               ValueMethod{120.0},
                                               true},
                  BacktestRunner::PositionRule{},
                  1000.0};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 2);
  ASSERT_FALSE(timeline.trade_records(last_timeline_index(timeline)).empty());
  EXPECT_TRUE(latest_record(timeline).is_closed_take_profit());
  EXPECT_DOUBLE_EQ(latest_record(timeline).exit_price(), 120.0);
}
