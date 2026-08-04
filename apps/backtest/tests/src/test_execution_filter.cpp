#include <gtest/gtest.h>

#include <array>
#include <vector>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

namespace {

auto make_filter_test_asset() -> Asset
{
  return Asset{"Filter test",
               AssetHistory{{"Datetime", {1.0, 2.0, 3.0, 4.0}},
                            {"Open", {100.0, 110.0, 120.0, 130.0}},
                            {"High", {100.0, 110.0, 120.0, 130.0}},
                            {"Low", {100.0, 110.0, 120.0, 130.0}},
                            {"Close", {100.0, 110.0, 120.0, 130.0}},
                            {"Volume", {0.0, 0.0, 0.0, 0.0}}}};
}

auto make_filter_test_rule() -> BacktestRunner::PositionRule
{
  auto signal_exits =
   std::vector<BacktestRunner::PositionRule::SignalExitRule>{};
  signal_exits.emplace_back(true,
                            EqualMethod{CloseMethod{}, ValueMethod{120.0}},
                            SignalTiming::CurrentClose,
                            1.0);
  return BacktestRunner::PositionRule{BooleanMethod<true>{},
                                      std::move(signal_exits),
                                      BooleanMethod<true>{},
                                      2,
                                      ValueMethod{10.0},
                                      {},
                                      SignalTiming::CurrentClose,
                                      SignalTiming::CurrentClose};
}

auto run_with_filter(
 ErasedSeriesMethod<ExecutionFilterMethodContext> execution_filter,
 PositionSizingNode position_sizing = PositionSizingNode{
  FixedQuantityPositionSizing{1.0}}) -> BacktestTimeline
{
  const auto asset = make_filter_test_asset();
  const auto market = Market{"Test", 0.0, 0.0};
  const auto broker = Broker{"Test"};
  const auto profile = Profile{"Test", std::move(position_sizing)};
  auto series_results = SeriesEvaluationResults{};
  auto timeline = BacktestTimeline{};
  auto runner = BacktestRunner{asset,
                               market,
                               broker,
                               profile,
                               {},
                               make_filter_test_rule(),
                               BacktestRunner::PositionRule{},
                               1'000.0,
                               0,
                               false,
                               1'000.0,
                               IntrabarPath::CandleDirection,
                               StrategyPerformanceConfig{},
                               std::move(execution_filter)};

  while(timeline.size() < asset.size()) {
    runner.run(series_results, timeline);
  }
  return timeline;
}

} // namespace

TEST(ExecutionFilterTest, RecordsOnlyIntentAndBooleanDecision)
{
  const auto decision = ExecutionFilterDecision{42, false};
  EXPECT_EQ(decision.intent_id(), 42);
  EXPECT_FALSE(decision.allowed());
}

TEST(ExecutionFilterTest, ProfileOwnsFilterAsAnExecutionRule)
{
  auto permissive = Profile{"Permissive"};
  auto restrictive = Profile{"Restrictive"};
  restrictive.execution_filter(
   ErasedNode<ExecutionFilterMethodContext>{FalseNode{}});

  EXPECT_NE(node_cast<TrueNode>(permissive.execution_filter()), nullptr);
  EXPECT_NE(node_cast<FalseNode>(restrictive.execution_filter()), nullptr);
  EXPECT_FALSE(permissive.equivalent_rules(restrictive));

  permissive.execution_filter(restrictive.execution_filter());
  EXPECT_TRUE(permissive.equivalent_rules(restrictive));
}

TEST(ExecutionFilterTest,
     RejectedInitialEntryStillCompletesShadowPositionAndStatistics)
{
  const auto timeline = run_with_filter(BooleanMethod<false>{});

  ASSERT_EQ(timeline.size(), 4);
  ASSERT_EQ(timeline.strategy_intents(0).size(), 1);
  EXPECT_EQ(timeline.strategy_intents(0).front().type(),
            StrategyIntentType::InitialEntry);
  ASSERT_EQ(timeline.execution_filter_decisions(0).size(), 1);
  EXPECT_FALSE(timeline.execution_filter_decisions(0).front().allowed());
  ASSERT_EQ(timeline.position_sizing_decisions(0).size(), 1);
  EXPECT_EQ(timeline.position_sizing_decisions(0).front().outcome,
            PositionSizingDecisionOutcome::Filtered);

  ASSERT_EQ(timeline.strategy_intents(1).size(), 1);
  EXPECT_EQ(timeline.strategy_intents(1).front().type(),
            StrategyIntentType::PyramidingEntry);
  EXPECT_TRUE(timeline.execution_filter_decisions(1).empty());
  ASSERT_EQ(timeline.position_sizing_decisions(1).size(), 1);
  EXPECT_EQ(timeline.position_sizing_decisions(1).front().outcome,
            PositionSizingDecisionOutcome::ShadowOnly);

  ASSERT_EQ(timeline.strategy_intents(2).size(), 1);
  EXPECT_EQ(timeline.strategy_intents(2).front().type(),
            StrategyIntentType::SignalExit);
  EXPECT_TRUE(timeline.execution_filter_decisions(2).empty());
  EXPECT_TRUE(timeline.position_sizing_decisions(2).empty());
  ASSERT_EQ(timeline.strategy_closed_positions(2).size(), 1);
  EXPECT_NEAR(timeline.strategy_closed_positions(2).front().return_ratio(),
              30.0 / 210.0,
              1e-12);
  EXPECT_EQ(timeline.strategy_performance(2).lifetime_count(), 1);

  for(auto index = std::size_t{0}; index < timeline.size(); ++index) {
    EXPECT_TRUE(timeline.trade_events(index).empty());
    EXPECT_TRUE(timeline.closed_trades(index).empty());
    EXPECT_FALSE(timeline.open_position(index).has_value());
  }
}

TEST(ExecutionFilterTest,
     AcceptedInitialEntryMirrorsPyramidingAndExitWithoutRefiltering)
{
  const auto timeline = run_with_filter(BooleanMethod<true>{});

  ASSERT_EQ(timeline.execution_filter_decisions(0).size(), 1);
  EXPECT_TRUE(timeline.execution_filter_decisions(0).front().allowed());
  EXPECT_TRUE(timeline.execution_filter_decisions(1).empty());
  EXPECT_TRUE(timeline.execution_filter_decisions(2).empty());
  ASSERT_EQ(timeline.position_sizing_decisions(0).size(), 1);
  EXPECT_EQ(timeline.position_sizing_decisions(0).front().outcome,
            PositionSizingDecisionOutcome::Executed);
  ASSERT_EQ(timeline.position_sizing_decisions(1).size(), 1);
  EXPECT_EQ(timeline.position_sizing_decisions(1).front().outcome,
            PositionSizingDecisionOutcome::Executed);
  ASSERT_TRUE(timeline.open_position(0).has_value());
  ASSERT_TRUE(timeline.open_position(1).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), 2.0);
  ASSERT_EQ(timeline.closed_trades(2).size(), 1);
  EXPECT_FALSE(timeline.open_position(2).has_value());
}

TEST(ExecutionFilterTest, LaterInitialEntryCanUseCompletedShadowPerformance)
{
  const auto filter = GreaterEqualMethod{
   StrategyPerformanceMethod{StrategyPerformanceMetric::LifetimeCount},
   ValueMethod{1.0}};
  const auto timeline = run_with_filter(filter);

  ASSERT_EQ(timeline.execution_filter_decisions(0).size(), 1);
  EXPECT_FALSE(timeline.execution_filter_decisions(0).front().allowed());
  ASSERT_EQ(timeline.execution_filter_decisions(3).size(), 1);
  EXPECT_TRUE(timeline.execution_filter_decisions(3).front().allowed());
  ASSERT_TRUE(timeline.open_position(3).has_value());
  EXPECT_DOUBLE_EQ(timeline.open_position(3)->entry_price(), 130.0);
}

TEST(ExecutionFilterTest, StrategyPerformanceMethodsExposeAllStreaks)
{
  const auto performance =
   StrategyPerformanceSnapshot{0,
                               1,
                               2,
                               3,
                               4,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               BayesianWinSnapshot{},
                               BayesianPayoffSnapshot{},
                               BayesianPayoffSnapshot{}};
  const auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  auto default_context = DefaultMethodContext{series_methods, series_results};
  const auto account_state = BacktestAccountState{};
  const auto base_context =
   BacktestMethodContext{default_context, series_methods, account_state};
  const auto context = ExecutionFilterMethodContext{base_context, performance};
  const auto asset = make_filter_test_asset();
  const auto snapshot = asset.get_snapshot(0);

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(
    StrategyPerformanceMethod{StrategyPerformanceMetric::CurrentWinningStreak},
    snapshot,
    context),
   1.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(
    StrategyPerformanceMethod{StrategyPerformanceMetric::CurrentLosingStreak},
    snapshot,
    context),
   2.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(
    StrategyPerformanceMethod{StrategyPerformanceMetric::MaximumWinningStreak},
    snapshot,
    context),
   3.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(
    StrategyPerformanceMethod{StrategyPerformanceMetric::MaximumLosingStreak},
    snapshot,
    context),
   4.0);
}

TEST(ExecutionFilterTest,
     BayesianKellyUsesCompletedTheoreticalPerformanceForLaterEntry)
{
  const auto timeline =
   run_with_filter(BooleanMethod<true>{},
                   PositionSizingNode{StrategyPerformanceBayesianKellySizing{
                    StrategyPerformanceBayesianKellyEstimate::PosteriorMean}});

  ASSERT_EQ(timeline.position_sizing_decisions(0).size(), 1U);
  EXPECT_EQ(timeline.position_sizing_decisions(0).front().outcome,
            PositionSizingDecisionOutcome::NoPositiveSize);
  ASSERT_EQ(timeline.position_sizing_decisions(1).size(), 1U);
  EXPECT_EQ(timeline.position_sizing_decisions(1).front().outcome,
            PositionSizingDecisionOutcome::ShadowOnly);
  ASSERT_EQ(timeline.strategy_performance(2).lifetime_count(), 1U);
  ASSERT_EQ(timeline.position_sizing_decisions(3).size(), 1U);
  const auto& decision = timeline.position_sizing_decisions(3).front();
  EXPECT_EQ(decision.outcome, PositionSizingDecisionOutcome::Executed);
  ASSERT_TRUE(decision.bayesian_kelly);
  EXPECT_GT(decision.bayesian_kelly->raw_kelly_fraction, 0.0);
  EXPECT_NEAR(*decision.primary_quantity, 1'000.0 / 130.0, 1e-12);
}
