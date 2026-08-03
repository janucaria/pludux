#include <cmath>
#include <limits>
#include <stdexcept>

#include <gtest/gtest.h>

import pludux.backtest;

namespace pludux::backtest::tests {
namespace {

auto context(double equity = 10'000.0,
             double entry_price = 100.0,
             double risk_distance = 5.0) -> PositionSizingContext
{
  static const auto performance = StrategyPerformance{}.snapshot();
  return PositionSizingContext{
   equity, entry_price, performance, [risk_distance] { return risk_distance; }};
}

auto context_with(const StrategyPerformanceSnapshot& performance,
                  double equity = 10'000.0,
                  double entry_price = 100.0) -> PositionSizingContext
{
  return PositionSizingContext{
   equity, entry_price, performance, [] { return 5.0; }};
}

auto performance_snapshot(double probability,
                          double win_mean,
                          double loss_mean,
                          double win_alpha = 6.0,
                          double win_beta = 4.0,
                          double payoff_shape = 3.0,
                          double payoff_scale = 1.0)
 -> StrategyPerformanceSnapshot
{
  return StrategyPerformanceSnapshot{
   0,
   0.0,
   0.0,
   0.0,
   0.0,
   0.0,
   0.0,
   BayesianWinSnapshot{.probability = probability,
                       .posterior_alpha = win_alpha,
                       .posterior_beta = win_beta},
   BayesianPayoffSnapshot{.mean = win_mean,
                          .posterior_shape = payoff_shape,
                          .posterior_scale = payoff_scale},
   BayesianPayoffSnapshot{.mean = loss_mean,
                          .posterior_shape = payoff_shape,
                          .posterior_scale = payoff_scale}};
}

TEST(PositionSizing, EvaluatesExistingMethods)
{
  EXPECT_DOUBLE_EQ(PositionSizingNode{RiskDistancePositionSizing{0.01}}
                    .make_method()
                    .evaluate(context())
                    .requested_quantity,
                   20.0);
  EXPECT_DOUBLE_EQ(PositionSizingNode{FixedQuantityPositionSizing{3.0}}
                    .make_method()
                    .evaluate(context())
                    .requested_quantity,
                   3.0);
  EXPECT_DOUBLE_EQ(PositionSizingNode{FixedNotionalPositionSizing{1'000.0}}
                    .make_method()
                    .evaluate(context())
                    .requested_quantity,
                   10.0);
  EXPECT_DOUBLE_EQ(PositionSizingNode{EquityFractionPositionSizing{0.10}}
                    .make_method()
                    .evaluate(context())
                    .requested_quantity,
                   10.0);
}

TEST(PositionSizing, IsValueSemanticAndTypeAware)
{
  const auto first = PositionSizingNode{FixedQuantityPositionSizing{3.0}};
  const auto copy = first;
  const auto different = PositionSizingNode{FixedQuantityPositionSizing{4.0}};
  const auto different_type =
   PositionSizingNode{FixedNotionalPositionSizing{3.0}};

  EXPECT_EQ(first, copy);
  EXPECT_NE(first, different);
  EXPECT_NE(first, different_type);
  ASSERT_NE(position_sizing_node_cast<FixedQuantityPositionSizing>(first),
            nullptr);
}

TEST(PositionSizing, RejectsInvalidConfiguration)
{
  EXPECT_THROW(RiskDistancePositionSizing{0.0}, std::invalid_argument);
  EXPECT_THROW(FixedQuantityPositionSizing{-1.0}, std::invalid_argument);
  EXPECT_THROW(
   FixedNotionalPositionSizing{std::numeric_limits<double>::infinity()},
   std::invalid_argument);
  EXPECT_THROW(EquityFractionPositionSizing{0.0}, std::invalid_argument);
  EXPECT_THROW(
   (StrategyPerformanceBayesianKellySizing{
    StrategyPerformanceBayesianKellyEstimate::AdverseQuantiles, 1.0, 0.5, 1.0}),
   std::invalid_argument);
  EXPECT_THROW((StrategyPerformanceBayesianKellySizing{
                StrategyPerformanceBayesianKellyEstimate::AdverseQuantiles,
                0.8,
                1.01,
                1.0}),
               std::invalid_argument);
}

TEST(PositionSizing, UntouchedPriorsProduceNoKellyPosition)
{
  const auto result =
   PositionSizingNode{StrategyPerformanceBayesianKellySizing{
                       StrategyPerformanceBayesianKellyEstimate::PosteriorMean}}
    .make_method()
    .evaluate(context());

  EXPECT_DOUBLE_EQ(result.requested_quantity, 0.0);
  ASSERT_TRUE(result.bayesian_kelly);
  EXPECT_NEAR(result.bayesian_kelly->win_probability, 0.5, 1e-12);
  EXPECT_NEAR(result.bayesian_kelly->winning_payoff, 0.01, 1e-12);
  EXPECT_NEAR(result.bayesian_kelly->losing_payoff, 0.01, 1e-12);
  EXPECT_NEAR(result.bayesian_kelly->raw_kelly_fraction, 0.0, 1e-12);
}

TEST(PositionSizing, PosteriorMeanAppliesMultiplierAndAllocationCap)
{
  const auto performance = performance_snapshot(0.6, 0.02, 0.01);
  const auto result =
   PositionSizingNode{
    StrategyPerformanceBayesianKellySizing{
     StrategyPerformanceBayesianKellyEstimate::PosteriorMean, 0.80, 0.50, 2.0}}
    .make_method()
    .evaluate(context_with(performance));

  ASSERT_TRUE(result.bayesian_kelly);
  EXPECT_NEAR(result.bayesian_kelly->raw_kelly_fraction, 40.0, 1e-12);
  EXPECT_NEAR(result.bayesian_kelly->scaled_kelly_fraction, 20.0, 1e-12);
  EXPECT_NEAR(result.bayesian_kelly->allocation_fraction, 2.0, 1e-12);
  EXPECT_NEAR(result.requested_quantity, 200.0, 1e-12);
}

TEST(PositionSizing, NegativeEdgeAndZeroMultiplierProduceNoPosition)
{
  const auto negative_performance = performance_snapshot(0.2, 0.02, 0.01);
  const auto negative =
   PositionSizingNode{StrategyPerformanceBayesianKellySizing{
                       StrategyPerformanceBayesianKellyEstimate::PosteriorMean}}
    .make_method()
    .evaluate(context_with(negative_performance));
  EXPECT_LT(negative.bayesian_kelly->raw_kelly_fraction, 0.0);
  EXPECT_DOUBLE_EQ(negative.requested_quantity, 0.0);

  const auto positive_performance = performance_snapshot(0.6, 0.02, 0.01);
  const auto disabled =
   PositionSizingNode{
    StrategyPerformanceBayesianKellySizing{
     StrategyPerformanceBayesianKellyEstimate::PosteriorMean, 0.80, 0.0, 1.0}}
    .make_method()
    .evaluate(context_with(positive_performance));
  EXPECT_GT(disabled.bayesian_kelly->raw_kelly_fraction, 0.0);
  EXPECT_DOUBLE_EQ(disabled.requested_quantity, 0.0);
}

TEST(PositionSizing, AdverseQuantilesUsesAdverseMarginalQuantiles)
{
  const auto performance =
   performance_snapshot(0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
  const auto result =
   PositionSizingNode{StrategyPerformanceBayesianKellySizing{}}
    .make_method()
    .evaluate(context_with(performance));

  ASSERT_TRUE(result.bayesian_kelly);
  EXPECT_NEAR(result.bayesian_kelly->win_probability, 0.1, 1e-12);
  EXPECT_NEAR(
   result.bayesian_kelly->winning_payoff, 1.0 / std::log(10.0), 1e-12);
  EXPECT_NEAR(
   result.bayesian_kelly->losing_payoff, 1.0 / -std::log(0.9), 1e-12);
  EXPECT_LT(result.bayesian_kelly->raw_kelly_fraction, 0.0);
  EXPECT_DOUBLE_EQ(result.requested_quantity, 0.0);
}

} // namespace
} // namespace pludux::backtest::tests
