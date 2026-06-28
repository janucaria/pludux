#include <gtest/gtest.h>

#include <vector>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

TEST(BacktestRunnerTest, RiskSizingUsesCurrentEquityAfterClosedTrade)
{
  const auto asset = Asset{
   "Test",
   AssetHistory{{"Datetime", {4.0, 3.0, 2.0, 1.0}},
                {"Open", {110.0, 110.0, 100.0, 50.0}},
                {"High", {110.0, 110.0, 100.0, 50.0}},
                {"Low", {110.0, 110.0, 100.0, 50.0}},
                {"Close", {110.0, 110.0, 100.0, 50.0}},
                {"Volume", {0.0, 0.0, 0.0, 0.0}}}};

  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile = Profile{"Test", 0.10};
  auto series_results = SeriesEvaluationResults{};
  auto summaries = std::vector<BacktestSummary>{};
  const auto entry_signal = LogicalOrMethod{
   EqualMethod{CloseMethod{}, ValueMethod{50.0}},
   EqualMethod{CloseMethod{}, ValueMethod{110.0}}};

  auto runner = BacktestRunner{
   asset,
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
