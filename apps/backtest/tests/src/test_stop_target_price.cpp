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
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  auto default_context = DefaultMethodContext{series_methods, series_results};
  const auto account_state = BacktestAccountState{};
  const auto context =
   BacktestMethodContext{default_context, series_methods, account_state, 0};
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
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  auto default_context = DefaultMethodContext{series_methods, series_results};
  const auto account_state = BacktestAccountState{};
  const auto context =
   BacktestMethodContext{default_context, series_methods, account_state, 0}
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
   node_to_erased_method<ErasedSeriesMethodContext>(Sl1RNode{}, node_context);
  const auto stop_loss_r_multiple =
   node_to_erased_method<ErasedSeriesMethodContext>(SlRMultipleNode{1.0},
                                                    node_context);

  EXPECT_EQ(stop_loss_1r, stop_loss_r_multiple);
}

} // namespace
} // namespace pludux::backtest::tests
