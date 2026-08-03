#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <stdexcept>

import pludux.backtest;

using namespace pludux::backtest;

namespace {

auto closed_position(std::size_t id,
                     StrategyDirection direction,
                     double return_ratio) -> StrategyClosedPosition
{
  constexpr auto notional = 100.0;
  return StrategyClosedPosition{
   id, direction, 10, 20, 1.0, notional, return_ratio * notional, {}};
}

class FixedWinModelNode {
public:
  explicit FixedWinModelNode(double probability) noexcept
  : probability_{probability}
  {
  }

  auto operator==(const FixedWinModelNode&) const noexcept -> bool = default;

  auto probability(this const FixedWinModelNode& self) noexcept -> double
  {
    return self.probability_;
  }

private:
  double probability_{};
};

class FixedWinModelMethod {
public:
  explicit FixedWinModelMethod(FixedWinModelNode node) noexcept
  : node_{node}
  {
  }

  auto evaluate(this const FixedWinModelMethod& self,
                const WeightedBinaryEvidence&) noexcept -> BayesianWinSnapshot
  {
    return BayesianWinSnapshot{self.node_.probability(),
                               self.node_.probability(),
                               self.node_.probability(),
                               1.0,
                               1.0};
  }

private:
  FixedWinModelNode node_;
};

auto node_to_model_method(const FixedWinModelNode& node) -> FixedWinModelMethod
{
  return FixedWinModelMethod{node};
}

class FixedPayoffModelNode {
public:
  explicit FixedPayoffModelNode(double mean) noexcept
  : mean_{mean}
  {
  }

  auto operator==(const FixedPayoffModelNode&) const noexcept -> bool = default;

  auto mean(this const FixedPayoffModelNode& self) noexcept -> double
  {
    return self.mean_;
  }

private:
  double mean_{};
};

class FixedPayoffModelMethod {
public:
  explicit FixedPayoffModelMethod(FixedPayoffModelNode node) noexcept
  : node_{node}
  {
  }

  auto evaluate(this const FixedPayoffModelMethod& self,
                const WeightedPayoffEvidence& evidence) noexcept
   -> BayesianPayoffSnapshot
  {
    return BayesianPayoffSnapshot{self.node_.mean(),
                                  self.node_.mean(),
                                  self.node_.mean(),
                                  1.0,
                                  self.node_.mean(),
                                  evidence.effective_count};
  }

private:
  FixedPayoffModelNode node_;
};

auto node_to_model_method(const FixedPayoffModelNode& node)
 -> FixedPayoffModelMethod
{
  return FixedPayoffModelMethod{node};
}

} // namespace

TEST(StrategyPerformanceTest, AggregatesMixedDirectionsIntoOneHistory)
{
  auto performance = StrategyPerformance{};
  performance.observe(closed_position(1, StrategyDirection::Long, 0.10));
  performance.observe(closed_position(2, StrategyDirection::Short, -0.05));

  const auto snapshot = performance.snapshot();
  EXPECT_EQ(snapshot.lifetime_count(), 2);
  EXPECT_DOUBLE_EQ(snapshot.effective_count(), 2.0);
  EXPECT_DOUBLE_EQ(snapshot.mean_return(), 0.025);
  EXPECT_DOUBLE_EQ(snapshot.win_rate(), 0.5);
  EXPECT_DOUBLE_EQ(snapshot.break_even_rate(), 0.0);
  EXPECT_DOUBLE_EQ(snapshot.loss_rate(), 0.5);
  EXPECT_DOUBLE_EQ(snapshot.winning_payoff_posterior().effective_count, 1.0);
  EXPECT_DOUBLE_EQ(snapshot.losing_payoff_posterior().effective_count, 1.0);
}

TEST(StrategyPerformanceTest, RollingWindowUsesRecentStrategyOutputs)
{
  const auto config =
   StrategyPerformanceConfig{StrategyPerformanceHistoryPolicy{
    StrategyPerformanceHistoryMode::RollingWindow, 2, 0.99}};
  auto performance = StrategyPerformance{config};
  performance.observe(closed_position(1, StrategyDirection::Long, 0.10));
  performance.observe(closed_position(2, StrategyDirection::Short, 0.20));
  performance.observe(closed_position(3, StrategyDirection::Long, -0.10));

  const auto snapshot = performance.snapshot();
  EXPECT_EQ(snapshot.lifetime_count(), 3);
  EXPECT_DOUBLE_EQ(snapshot.effective_count(), 2.0);
  EXPECT_NEAR(snapshot.mean_return(), 0.05, 1e-12);
  EXPECT_DOUBLE_EQ(snapshot.win_rate(), 0.5);
  EXPECT_DOUBLE_EQ(snapshot.break_even_rate(), 0.0);
  EXPECT_DOUBLE_EQ(snapshot.loss_rate(), 0.5);
  EXPECT_DOUBLE_EQ(snapshot.winning_payoff_posterior().effective_count, 1.0);
  EXPECT_DOUBLE_EQ(snapshot.losing_payoff_posterior().effective_count, 1.0);
}

TEST(StrategyPerformanceTest, DefaultBayesianPriorsAreWeakAndNeutral)
{
  const auto snapshot = StrategyPerformance{}.snapshot();
  const auto& win = snapshot.win_probability_posterior();
  const auto& winning = snapshot.winning_payoff_posterior();
  const auto& losing = snapshot.losing_payoff_posterior();

  EXPECT_DOUBLE_EQ(win.probability, 0.5);
  EXPECT_DOUBLE_EQ(win.lower_95, 0.025);
  EXPECT_DOUBLE_EQ(win.upper_95, 0.975);
  EXPECT_DOUBLE_EQ(win.posterior_alpha, 1.0);
  EXPECT_DOUBLE_EQ(win.posterior_beta, 1.0);
  EXPECT_NEAR(winning.mean, 0.01, 1e-12);
  EXPECT_NEAR(losing.mean, 0.01, 1e-12);
  EXPECT_DOUBLE_EQ(winning.posterior_shape, 1.01);
  EXPECT_DOUBLE_EQ(winning.posterior_scale, 0.0001);
  EXPECT_LT(0.0, winning.lower_95);
  EXPECT_LT(winning.lower_95, winning.upper_95);
}

TEST(StrategyPerformanceTest, BayesianPosteriorsMatchReferenceValues)
{
  auto performance = StrategyPerformance{};
  performance.observe(closed_position(1, StrategyDirection::Long, 0.10));
  performance.observe(closed_position(2, StrategyDirection::Short, 0.20));
  performance.observe(closed_position(3, StrategyDirection::Long, -0.10));

  const auto snapshot = performance.snapshot();
  const auto& win = snapshot.win_probability_posterior();
  EXPECT_DOUBLE_EQ(win.probability, 0.6);
  EXPECT_NEAR(win.lower_95, 0.19412044968324338, 1e-12);
  EXPECT_NEAR(win.upper_95, 0.9324140135114569, 1e-12);
  EXPECT_DOUBLE_EQ(win.posterior_alpha, 3.0);
  EXPECT_DOUBLE_EQ(win.posterior_beta, 2.0);

  const auto& winning = snapshot.winning_payoff_posterior();
  EXPECT_DOUBLE_EQ(winning.effective_count, 2.0);
  EXPECT_DOUBLE_EQ(winning.posterior_shape, 3.01);
  EXPECT_NEAR(winning.posterior_scale, 0.3001, 1e-12);
  EXPECT_NEAR(winning.mean, 0.3001 / 2.01, 1e-12);

  const auto& losing = snapshot.losing_payoff_posterior();
  EXPECT_DOUBLE_EQ(losing.effective_count, 1.0);
  EXPECT_DOUBLE_EQ(losing.posterior_shape, 2.01);
  EXPECT_NEAR(losing.posterior_scale, 0.1001, 1e-12);
  EXPECT_NEAR(losing.mean, 0.1001 / 1.01, 1e-12);
}

TEST(StrategyPerformanceTest, ExponentialDecayAgesEveryEvidenceStream)
{
  const auto config =
   StrategyPerformanceConfig{StrategyPerformanceHistoryPolicy{
    StrategyPerformanceHistoryMode::ExponentialDecay, 100, 0.5}};
  auto performance = StrategyPerformance{config};
  performance.observe(closed_position(1, StrategyDirection::Long, 0.10));
  performance.observe(closed_position(2, StrategyDirection::Short, -0.10));

  const auto snapshot = performance.snapshot();
  EXPECT_EQ(snapshot.lifetime_count(), 2);
  EXPECT_DOUBLE_EQ(snapshot.effective_count(), 1.5);
  EXPECT_NEAR(snapshot.win_rate(), 1.0 / 3.0, 1e-12);
  EXPECT_DOUBLE_EQ(snapshot.break_even_rate(), 0.0);
  EXPECT_NEAR(snapshot.loss_rate(), 2.0 / 3.0, 1e-12);
  EXPECT_NEAR(snapshot.mean_return(), -1.0 / 30.0, 1e-12);
  EXPECT_NEAR(
   snapshot.win_probability_posterior().probability, 3.0 / 7.0, 1e-12);
  EXPECT_DOUBLE_EQ(snapshot.winning_payoff_posterior().effective_count, 0.5);
  EXPECT_DOUBLE_EQ(snapshot.losing_payoff_posterior().effective_count, 1.0);
  EXPECT_NEAR(snapshot.winning_payoff_posterior().mean, 0.0501 / 0.51, 1e-12);
  EXPECT_NEAR(snapshot.losing_payoff_posterior().mean, 0.1001 / 1.01, 1e-12);
}

TEST(StrategyPerformanceTest, BreakEvenDefaultsToLossClassification)
{
  auto performance = StrategyPerformance{};
  performance.observe(closed_position(1, StrategyDirection::Long, 0.0));

  const auto snapshot = performance.snapshot();
  EXPECT_EQ(snapshot.lifetime_count(), 1);
  EXPECT_DOUBLE_EQ(snapshot.effective_count(), 1.0);
  EXPECT_DOUBLE_EQ(snapshot.win_rate(), 0.0);
  EXPECT_DOUBLE_EQ(snapshot.break_even_rate(), 1.0);
  EXPECT_DOUBLE_EQ(snapshot.loss_rate(), 0.0);
  EXPECT_DOUBLE_EQ(snapshot.mean_return(), 0.0);
  EXPECT_DOUBLE_EQ(snapshot.return_standard_deviation(), 0.0);
  EXPECT_DOUBLE_EQ(snapshot.win_probability_posterior().probability, 1.0 / 3.0);
  EXPECT_DOUBLE_EQ(snapshot.winning_payoff_posterior().effective_count, 0.0);
  EXPECT_DOUBLE_EQ(snapshot.losing_payoff_posterior().effective_count, 0.0);
  EXPECT_NEAR(snapshot.winning_payoff_posterior().mean, 0.01, 1e-12);
  EXPECT_NEAR(snapshot.losing_payoff_posterior().mean, 0.01, 1e-12);
}

TEST(StrategyPerformanceTest, BreakEvenTreatmentOnlyChangesBinaryEvidence)
{
  struct Expected {
    StrategyPerformanceBreakEvenTreatment treatment;
    double posterior_probability;
  };
  constexpr auto cases = std::array{
   Expected{StrategyPerformanceBreakEvenTreatment::Skip, 0.5},
   Expected{StrategyPerformanceBreakEvenTreatment::CountAsWin, 2.0 / 3.0},
   Expected{StrategyPerformanceBreakEvenTreatment::CountAsLoss, 1.0 / 3.0}};

  for(const auto& expected : cases) {
    SCOPED_TRACE(static_cast<int>(expected.treatment));
    auto performance =
     StrategyPerformance{StrategyPerformanceConfig{{}, {}, expected.treatment}};
    performance.observe(closed_position(1, StrategyDirection::Long, 0.0));

    const auto snapshot = performance.snapshot();
    EXPECT_EQ(snapshot.lifetime_count(), 1);
    EXPECT_DOUBLE_EQ(snapshot.effective_count(), 1.0);
    EXPECT_DOUBLE_EQ(snapshot.mean_return(), 0.0);
    EXPECT_DOUBLE_EQ(snapshot.win_rate(), 0.0);
    EXPECT_DOUBLE_EQ(snapshot.break_even_rate(), 1.0);
    EXPECT_DOUBLE_EQ(snapshot.loss_rate(), 0.0);
    EXPECT_DOUBLE_EQ(snapshot.win_probability_posterior().probability,
                     expected.posterior_probability);
    EXPECT_DOUBLE_EQ(snapshot.winning_payoff_posterior().effective_count, 0.0);
    EXPECT_DOUBLE_EQ(snapshot.losing_payoff_posterior().effective_count, 0.0);
  }
}

TEST(StrategyPerformanceTest, BreakEvenOccupiesRollingWindow)
{
  struct Expected {
    StrategyPerformanceBreakEvenTreatment treatment;
    double posterior_probability;
  };
  constexpr auto cases = std::array{
   Expected{StrategyPerformanceBreakEvenTreatment::Skip, 1.0 / 3.0},
   Expected{StrategyPerformanceBreakEvenTreatment::CountAsWin, 0.5},
   Expected{StrategyPerformanceBreakEvenTreatment::CountAsLoss, 0.25}};

  for(const auto& expected : cases) {
    SCOPED_TRACE(static_cast<int>(expected.treatment));
    auto performance = StrategyPerformance{StrategyPerformanceConfig{
     StrategyPerformanceHistoryPolicy{
      StrategyPerformanceHistoryMode::RollingWindow, 2, 0.99},
     {},
     expected.treatment}};
    performance.observe(closed_position(1, StrategyDirection::Long, 0.10));
    performance.observe(closed_position(2, StrategyDirection::Long, 0.0));
    performance.observe(closed_position(3, StrategyDirection::Long, -0.10));

    const auto snapshot = performance.snapshot();
    EXPECT_EQ(snapshot.lifetime_count(), 3);
    EXPECT_DOUBLE_EQ(snapshot.effective_count(), 2.0);
    EXPECT_NEAR(snapshot.mean_return(), -0.05, 1e-12);
    EXPECT_DOUBLE_EQ(snapshot.win_rate(), 0.0);
    EXPECT_DOUBLE_EQ(snapshot.break_even_rate(), 0.5);
    EXPECT_DOUBLE_EQ(snapshot.loss_rate(), 0.5);
    EXPECT_DOUBLE_EQ(snapshot.win_probability_posterior().probability,
                     expected.posterior_probability);
    EXPECT_DOUBLE_EQ(snapshot.winning_payoff_posterior().effective_count, 0.0);
    EXPECT_DOUBLE_EQ(snapshot.losing_payoff_posterior().effective_count, 1.0);
  }
}

TEST(StrategyPerformanceTest, BreakEvenAgesExponentialEvidence)
{
  auto performance = StrategyPerformance{StrategyPerformanceConfig{
   StrategyPerformanceHistoryPolicy{
    StrategyPerformanceHistoryMode::ExponentialDecay, 100, 0.5},
   {},
   StrategyPerformanceBreakEvenTreatment::CountAsLoss}};
  performance.observe(closed_position(1, StrategyDirection::Long, 0.10));
  performance.observe(closed_position(2, StrategyDirection::Long, 0.0));

  const auto snapshot = performance.snapshot();
  EXPECT_DOUBLE_EQ(snapshot.effective_count(), 1.5);
  EXPECT_NEAR(snapshot.win_rate(), 1.0 / 3.0, 1e-12);
  EXPECT_NEAR(snapshot.break_even_rate(), 2.0 / 3.0, 1e-12);
  EXPECT_DOUBLE_EQ(snapshot.loss_rate(), 0.0);
  EXPECT_NEAR(
   snapshot.win_probability_posterior().probability, 3.0 / 7.0, 1e-12);
  EXPECT_DOUBLE_EQ(snapshot.winning_payoff_posterior().effective_count, 0.5);
  EXPECT_DOUBLE_EQ(snapshot.losing_payoff_posterior().effective_count, 0.0);
}

TEST(StrategyPerformanceTest, BreakEvenTreatmentHasValueSemantics)
{
  const auto skipped = StrategyPerformanceConfig{
   {}, {}, StrategyPerformanceBreakEvenTreatment::Skip};
  auto counted_as_loss = StrategyPerformanceConfig{};

  EXPECT_NE(skipped, counted_as_loss);
  counted_as_loss.break_even_treatment(
   StrategyPerformanceBreakEvenTreatment::Skip);
  EXPECT_EQ(skipped, counted_as_loss);
  EXPECT_THROW(counted_as_loss.break_even_treatment(
                static_cast<StrategyPerformanceBreakEvenTreatment>(-1)),
               std::invalid_argument);
}

TEST(StrategyPerformanceTest, AlternativeModelsUseCommonSnapshotContracts)
{
  const auto config = StrategyPerformanceConfig{
   {},
   StrategyPerformanceBayesianConfig{
    BayesianWinModelNode{FixedWinModelNode{0.8}},
    BayesianPayoffModelNode{FixedPayoffModelNode{0.03}},
    BayesianPayoffModelNode{FixedPayoffModelNode{0.04}}}};
  const auto snapshot = StrategyPerformance{config}.snapshot();

  EXPECT_DOUBLE_EQ(snapshot.win_probability_posterior().probability, 0.8);
  EXPECT_DOUBLE_EQ(snapshot.winning_payoff_posterior().mean, 0.03);
  EXPECT_DOUBLE_EQ(snapshot.losing_payoff_posterior().mean, 0.04);
  EXPECT_NE(bayesian_model_node_cast<FixedWinModelNode>(
             config.bayesian().win_probability_model()),
            nullptr);
  EXPECT_NE(bayesian_model_node_cast<FixedPayoffModelNode>(
             config.bayesian().winning_payoff_model()),
            nullptr);
  EXPECT_NE(bayesian_model_node_cast<FixedPayoffModelNode>(
             config.bayesian().losing_payoff_model()),
            nullptr);
}

TEST(StrategyPerformanceTest, RejectsInvalidHistoryAndPriors)
{
  EXPECT_THROW((StrategyPerformanceConfig{StrategyPerformanceHistoryPolicy{
                StrategyPerformanceHistoryMode::RollingWindow, 0, 0.99}}),
               std::invalid_argument);
  EXPECT_THROW((StrategyPerformanceBayesianConfig{
                BayesianWinModelNode{BetaBernoulliModelNode{1.0, 2.0}},
                BayesianPayoffModelNode{GammaPayoffModelNode{}},
                BayesianPayoffModelNode{GammaPayoffModelNode{}}}),
               std::invalid_argument);
  EXPECT_THROW((StrategyPerformanceBayesianConfig{
                BayesianWinModelNode{BetaBernoulliModelNode{}},
                BayesianPayoffModelNode{GammaPayoffModelNode{0.0, 0.01, 1.0}},
                BayesianPayoffModelNode{GammaPayoffModelNode{}}}),
               std::invalid_argument);
  EXPECT_THROW(
   (StrategyPerformanceBayesianConfig{
    BayesianWinModelNode{BetaBernoulliModelNode{}},
    BayesianPayoffModelNode{GammaPayoffModelNode{}},
    BayesianPayoffModelNode{GammaPayoffModelNode{0.01, 0.01, 0.0}}}),
   std::invalid_argument);
}
