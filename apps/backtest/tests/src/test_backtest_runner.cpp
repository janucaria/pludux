#include <gtest/gtest.h>

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

auto run_single_entry(PositionSizing position_sizing,
                      double entry_price = 100.0) -> BacktestSummary
{
  const auto asset = make_single_bar_asset(entry_price);
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile = Profile{"Test", position_sizing};
  auto series_results = SeriesEvaluationResults{};
  auto summaries = std::vector<BacktestSummary>{};

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
                  1000.0};

  runner.run(series_results, summaries);

  return summaries.back();
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
  auto summaries = std::vector<BacktestSummary>{};
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

  runner.run(series_results, summaries);
  runner.run(series_results, summaries);

  ASSERT_EQ(summaries.size(), 2);
  ASSERT_TRUE(summaries.back().trade_session().open_position().has_value());
  EXPECT_DOUBLE_EQ(
   summaries.back().trade_session().open_position()->position_size(), 10.0);

  runner.run(series_results, summaries);

  ASSERT_EQ(summaries.size(), 3);
  EXPECT_FALSE(summaries.back().trade_session().open_position().has_value());
  EXPECT_DOUBLE_EQ(summaries.back().equity(), 1100.0);

  runner.run(series_results, summaries);

  ASSERT_EQ(summaries.size(), 4);
  ASSERT_TRUE(summaries.back().trade_session().open_position().has_value());
  EXPECT_DOUBLE_EQ(
   summaries.back().trade_session().open_position()->position_size(), 5.5);
}

TEST(BacktestRunnerTest, FixedQuantitySizingUsesExactQuantity)
{
  const auto summary =
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedQuantity, 12.5});

  ASSERT_TRUE(summary.trade_session().open_position().has_value());
  EXPECT_DOUBLE_EQ(summary.trade_session().open_position()->position_size(),
                   12.5);
}

TEST(BacktestRunnerTest, FixedNotionalSizingConvertsByEntryPrice)
{
  const auto summary =
   run_single_entry(PositionSizing{PositionSizing::Mode::FixedNotional, 250.0});

  ASSERT_TRUE(summary.trade_session().open_position().has_value());
  EXPECT_DOUBLE_EQ(summary.trade_session().open_position()->position_size(),
                   2.5);
}

TEST(BacktestRunnerTest, EquityPercentSizingConvertsByCurrentEquity)
{
  const auto summary =
   run_single_entry(PositionSizing{PositionSizing::Mode::EquityPercent, 0.25});

  ASSERT_TRUE(summary.trade_session().open_position().has_value());
  EXPECT_DOUBLE_EQ(summary.trade_session().open_position()->position_size(),
                   2.5);
}
