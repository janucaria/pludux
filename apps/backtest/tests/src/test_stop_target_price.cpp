#include <array>
#include <cmath>
#include <limits>

#include <gtest/gtest.h>

import pludux.backtest;

namespace pludux::backtest::tests {
namespace {

TEST(StopTargetPrice, EvaluatesStopLossRMultiplesForLongAndShortPositions)
{
  const auto snapshot = AssetSnapshot{AssetHistory{{"close", {100.0}}}};
  const auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<BacktestMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  const auto account_state = BacktestAccountState{};
  const auto context =
   BacktestMethodContext{series_methods, series_results, 0, account_state, 0};
  const auto long_context = context.with_position_reference(100.0, 1.0)
                             .with_position_risk_distance(10.0);
  const auto short_context = context.with_position_reference(100.0, -1.0)
                              .with_position_risk_distance(10.0);

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(SlRMultipleMethod{}, snapshot, long_context), 90.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(SlRMultipleMethod{2.0}, snapshot, long_context),
   80.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(SlRMultipleMethod{2.0}, snapshot, short_context),
   120.0);

  const auto configurable_multiple =
   AddMethod{ValueMethod{0.5}, ValueMethod{1.5}};
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(
    SlRMultipleMethod{configurable_multiple}, snapshot, long_context),
   80.0);
}

TEST(StopTargetPrice, RejectsInvalidStopLossRMultiples)
{
  const auto snapshot = AssetSnapshot{AssetHistory{{"close", {100.0}}}};
  const auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<BacktestMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  const auto account_state = BacktestAccountState{};
  const auto context =
   BacktestMethodContext{series_methods, series_results, 0, account_state, 0}
    .with_position_reference(100.0, 1.0)
    .with_position_risk_distance(10.0);
  const auto invalid_multiples =
   std::array{0.0,
              -1.0,
              std::numeric_limits<double>::quiet_NaN(),
              std::numeric_limits<double>::infinity()};

  for(const auto multiple : invalid_multiples) {
    EXPECT_TRUE(std::isnan(
     evaluate_series_method(SlRMultipleMethod{multiple}, snapshot, context)));
  }
}

TEST(StopTargetPrice, StopLoss1RDelegatesToOneRMultipleNode)
{
  auto node_context = NodeToErasedMethodContext{};
  const auto stop_loss_1r =
   node_to_erased_method<BacktestMethodContext>(Sl1RNode{}, node_context);
  const auto stop_loss_r_multiple =
   node_to_erased_method<BacktestMethodContext>(
    SlRMultipleNode<BacktestMethodContext>{1.0},
                                                    node_context);

  EXPECT_EQ(stop_loss_1r, stop_loss_r_multiple);
}

TEST(StopTargetPrice, TypedNodesComposeForBacktestMethodContext)
{
  const auto snapshot = AssetSnapshot{AssetHistory{{"Close", {100.0}}}};
  const auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<BacktestMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  const auto account_state = BacktestAccountState{};
  const auto context =
   BacktestMethodContext{series_methods, series_results, 0, account_state, 3}
    .with_position_reference(100.0, 1.0)
    .with_position_risk_distance(10.0);
  auto conversion_context = NodeToErasedMethodContext{};

  const auto stop_loss = node_to_erased_method<BacktestMethodContext>(
   SlAmountNode<BacktestMethodContext>{
    ErasedNode<BacktestMethodContext>{ValueNode{10.0}}},
   conversion_context);
  const auto risk_distance = node_to_erased_method<BacktestMethodContext>(
   RiskDistancePercentNode<BacktestMethodContext>{
    ErasedNode<BacktestMethodContext>{ValueNode{5.0}}},
   conversion_context);
  const auto r_multiple = node_to_erased_method<BacktestMethodContext>(
   PositionRMultipleNode<BacktestMethodContext>{
    ErasedNode<BacktestMethodContext>{ValueNode{100.0}}},
   conversion_context);
  const auto pyramiding_layer = node_to_erased_method<BacktestMethodContext>(
   PyramidingLayerNode{}, conversion_context);

  EXPECT_DOUBLE_EQ(evaluate_series_method(stop_loss, snapshot, context), 90.0);
  EXPECT_DOUBLE_EQ(evaluate_series_method(risk_distance, snapshot, context),
                   5.0);
  EXPECT_DOUBLE_EQ(evaluate_series_method(r_multiple, snapshot, context), 0.0);
  EXPECT_DOUBLE_EQ(evaluate_series_method(pyramiding_layer, snapshot, context),
                   3.0);
}

TEST(StopTargetPrice, SelectOutputNodeConvertsAndEvaluatesForBacktestContext)
{
  const auto snapshot = AssetSnapshot{AssetHistory{{"Close", {100.0}}}};
  const auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<BacktestMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  const auto account_state = BacktestAccountState{};
  const auto context =
   BacktestMethodContext{series_methods, series_results, 0, account_state, 0};
  const auto node = SelectOutputNode<BacktestMethodContext>{
   ErasedNode<BacktestMethodContext>{BbNode<BacktestMethodContext>{
    ErasedNode<BacktestMethodContext>{ValueNode{100.0}},
    ErasedNode<BacktestMethodContext>{ValueNode{1.0}},
    ErasedNode<BacktestMethodContext>{ValueNode{1.0}}}},
   NodeOutput::UpperBand};
  auto conversion_context = NodeToErasedMethodContext{};

  const auto method =
   node_to_erased_method<BacktestMethodContext>(node, conversion_context);
  const auto* select_output = series_method_cast<
   SelectOutputMethod<ErasedSeriesMethod<BacktestMethodContext>>>(method);

  ASSERT_NE(select_output, nullptr);
  EXPECT_EQ(select_output->output(), MethodOutput::UpperBand);
  const auto expected = evaluate_series_method(
   SelectOutputMethod{BbMethod{ValueMethod{100.0}, 1, 1.0},
                      MethodOutput::UpperBand},
   snapshot,
   context);
  EXPECT_DOUBLE_EQ(evaluate_series_method(method, snapshot, context), expected);
}

} // namespace
} // namespace pludux::backtest::tests
