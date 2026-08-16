#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <stdexcept>

import pludux.backtest;

using namespace pludux::backtest;

namespace {

auto closed_position(std::size_t id,
                      ModelDirection direction,
                      double return_ratio) -> ModelClosedPosition
{
  constexpr auto notional = 100.0;
  return ModelClosedPosition{
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

TEST(ModelPerformanceTest, AggregatesMixedDirectionsIntoOneHistory)
{
  auto performance = ModelPerformance{};
  performance.observe(closed_position(1, ModelDirection::Long, 0.10));
  performance.observe(closed_position(2, ModelDirection::Short, -0.05));

  const auto snapshot = performance.snapshot();
  EXPECT_EQ(snapshot.lifetime_count(), 2);
  EXPECT_DOUBLE_EQ(snapshot.effective_count(), 2.0);
  EXPECT_DOUBLE_EQ(snapshot.mean_return(), 0.025);
  EXPECT_DOUBLE_EQ(snapshot.win_rate(), 0.5);
  EXPECT_DOUBLE_EQ(snapshot.break_even_rate(), 0.0);
  EXPECT_DOUBLE_EQ(snapshot.loss_rate(), 0.5);
  EXPECT_EQ(snapshot.current_winning_streak(), 0);
  EXPECT_EQ(snapshot.current_losing_streak(), 1);
  EXPECT_EQ(snapshot.maximum_winning_streak(), 1);
  EXPECT_EQ(snapshot.maximum_losing_streak(), 1);
  EXPECT_DOUBLE_EQ(snapshot.winning_payoff_posterior().effective_count, 1.0);
  EXPECT_DOUBLE_EQ(snapshot.losing_payoff_posterior().effective_count, 1.0);
}

TEST(ModelPerformanceTest, TracksLifetimeWinningAndLosingStreaks)
{
  auto performance = ModelPerformance{};
  performance.observe(closed_position(1, ModelDirection::Long, 0.10));
  performance.observe(closed_position(2, ModelDirection::Short, 0.20));
  performance.observe(closed_position(3, ModelDirection::Long, -0.10));
  performance.observe(closed_position(4, ModelDirection::Short, -0.20));
  performance.observe(closed_position(5, ModelDirection::Long, -0.30));
  performance.observe(closed_position(6, ModelDirection::Short, 0.0));
  performance.observe(closed_position(7, ModelDirection::Long, 0.10));

  const auto snapshot = performance.snapshot();
  EXPECT_EQ(snapshot.current_winning_streak(), 1);
  EXPECT_EQ(snapshot.current_losing_streak(), 0);
  EXPECT_EQ(snapshot.maximum_winning_streak(), 2);
  EXPECT_EQ(snapshot.maximum_losing_streak(), 3);
}

TEST(ModelPerformanceTest, StreaksIgnoreStatisticalHistoryMode)
{
  const auto histories =
   std::array{ModelPerformanceHistoryPolicy{
               ModelPerformanceHistoryMode::All, 100, 0.99},
              ModelPerformanceHistoryPolicy{
               ModelPerformanceHistoryMode::RollingWindow, 2, 0.99},
              ModelPerformanceHistoryPolicy{
               ModelPerformanceHistoryMode::ExponentialDecay, 100, 0.5}};

  for(const auto& history : histories) {
    SCOPED_TRACE(static_cast<int>(history.mode()));
    auto performance = ModelPerformance{ModelPerformanceConfig{history}};
    performance.observe(closed_position(1, ModelDirection::Long, 0.10));
    performance.observe(closed_position(2, ModelDirection::Short, 0.20));
    performance.observe(closed_position(3, ModelDirection::Long, -0.10));
    performance.observe(closed_position(4, ModelDirection::Short, -0.20));
    performance.observe(closed_position(5, ModelDirection::Long, -0.30));

    const auto snapshot = performance.snapshot();
    EXPECT_EQ(snapshot.current_winning_streak(), 0);
    EXPECT_EQ(snapshot.current_losing_streak(), 3);
    EXPECT_EQ(snapshot.maximum_winning_streak(), 2);
    EXPECT_EQ(snapshot.maximum_losing_streak(), 3);
  }
}

TEST(ModelPerformanceTest,
     BreakEvenResetsStreaksRegardlessOfBayesianTreatment)
{
  constexpr auto treatments =
   std::array{ModelPerformanceBreakEvenTreatment::Skip,
              ModelPerformanceBreakEvenTreatment::CountAsWin,
              ModelPerformanceBreakEvenTreatment::CountAsLoss};

  for(const auto treatment : treatments) {
    SCOPED_TRACE(static_cast<int>(treatment));
    auto performance =
     ModelPerformance{ModelPerformanceConfig{{}, {}, treatment}};
    performance.observe(closed_position(1, ModelDirection::Long, 0.10));
    performance.observe(closed_position(2, ModelDirection::Short, 0.20));
    performance.observe(closed_position(3, ModelDirection::Long, 0.0));

    const auto snapshot = performance.snapshot();
    EXPECT_EQ(snapshot.current_winning_streak(), 0);
    EXPECT_EQ(snapshot.current_losing_streak(), 0);
    EXPECT_EQ(snapshot.maximum_winning_streak(), 2);
    EXPECT_EQ(snapshot.maximum_losing_streak(), 0);
  }
}

TEST(ModelPerformanceTest, RollingWindowUsesRecentModelOutputs)
{
  const auto config =
   ModelPerformanceConfig{ModelPerformanceHistoryPolicy{
    ModelPerformanceHistoryMode::RollingWindow, 2, 0.99}};
  auto performance = ModelPerformance{config};
  performance.observe(closed_position(1, ModelDirection::Long, 0.10));
  performance.observe(closed_position(2, ModelDirection::Short, 0.20));
  performance.observe(closed_position(3, ModelDirection::Long, -0.10));

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

TEST(ModelPerformanceTest, DefaultBayesianPriorsAreWeakAndNeutral)
{
  const auto snapshot = ModelPerformance{}.snapshot();
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

TEST(ModelPerformanceTest, BayesianPosteriorsMatchReferenceValues)
{
  auto performance = ModelPerformance{};
  performance.observe(closed_position(1, ModelDirection::Long, 0.10));
  performance.observe(closed_position(2, ModelDirection::Short, 0.20));
  performance.observe(closed_position(3, ModelDirection::Long, -0.10));

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

TEST(ModelPerformanceTest, ExponentialDecayAgesEveryEvidenceStream)
{
  const auto config =
   ModelPerformanceConfig{ModelPerformanceHistoryPolicy{
    ModelPerformanceHistoryMode::ExponentialDecay, 100, 0.5}};
  auto performance = ModelPerformance{config};
  performance.observe(closed_position(1, ModelDirection::Long, 0.10));
  performance.observe(closed_position(2, ModelDirection::Short, -0.10));

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

TEST(ModelPerformanceTest, BreakEvenDefaultsToLossClassification)
{
  auto performance = ModelPerformance{};
  performance.observe(closed_position(1, ModelDirection::Long, 0.0));

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

TEST(ModelPerformanceTest, BreakEvenTreatmentOnlyChangesBinaryEvidence)
{
  struct Expected {
    ModelPerformanceBreakEvenTreatment treatment;
    double posterior_probability;
  };
  constexpr auto cases = std::array{
   Expected{ModelPerformanceBreakEvenTreatment::Skip, 0.5},
   Expected{ModelPerformanceBreakEvenTreatment::CountAsWin, 2.0 / 3.0},
   Expected{ModelPerformanceBreakEvenTreatment::CountAsLoss, 1.0 / 3.0}};

  for(const auto& expected : cases) {
    SCOPED_TRACE(static_cast<int>(expected.treatment));
    auto performance =
     ModelPerformance{ModelPerformanceConfig{{}, {}, expected.treatment}};
    performance.observe(closed_position(1, ModelDirection::Long, 0.0));

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

TEST(ModelPerformanceTest, BreakEvenOccupiesRollingWindow)
{
  struct Expected {
    ModelPerformanceBreakEvenTreatment treatment;
    double posterior_probability;
  };
  constexpr auto cases = std::array{
   Expected{ModelPerformanceBreakEvenTreatment::Skip, 1.0 / 3.0},
   Expected{ModelPerformanceBreakEvenTreatment::CountAsWin, 0.5},
   Expected{ModelPerformanceBreakEvenTreatment::CountAsLoss, 0.25}};

  for(const auto& expected : cases) {
    SCOPED_TRACE(static_cast<int>(expected.treatment));
    auto performance = ModelPerformance{ModelPerformanceConfig{
     ModelPerformanceHistoryPolicy{
      ModelPerformanceHistoryMode::RollingWindow, 2, 0.99},
     {},
     expected.treatment}};
    performance.observe(closed_position(1, ModelDirection::Long, 0.10));
    performance.observe(closed_position(2, ModelDirection::Long, 0.0));
    performance.observe(closed_position(3, ModelDirection::Long, -0.10));

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

TEST(ModelPerformanceTest, BreakEvenAgesExponentialEvidence)
{
  auto performance = ModelPerformance{ModelPerformanceConfig{
   ModelPerformanceHistoryPolicy{
    ModelPerformanceHistoryMode::ExponentialDecay, 100, 0.5},
   {},
   ModelPerformanceBreakEvenTreatment::CountAsLoss}};
  performance.observe(closed_position(1, ModelDirection::Long, 0.10));
  performance.observe(closed_position(2, ModelDirection::Long, 0.0));

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

TEST(ModelPerformanceTest, BreakEvenTreatmentHasValueSemantics)
{
  const auto skipped = ModelPerformanceConfig{
   {}, {}, ModelPerformanceBreakEvenTreatment::Skip};
  auto counted_as_loss = ModelPerformanceConfig{};

  EXPECT_NE(skipped, counted_as_loss);
  counted_as_loss.break_even_treatment(
   ModelPerformanceBreakEvenTreatment::Skip);
  EXPECT_EQ(skipped, counted_as_loss);
  EXPECT_THROW(counted_as_loss.break_even_treatment(
                static_cast<ModelPerformanceBreakEvenTreatment>(-1)),
               std::invalid_argument);
}

TEST(ModelPerformanceTest, AlternativeModelsUseCommonSnapshotContracts)
{
  const auto config = ModelPerformanceConfig{
   {},
   ModelPerformanceBayesianConfig{
    BayesianWinModelNode{FixedWinModelNode{0.8}},
    BayesianPayoffModelNode{FixedPayoffModelNode{0.03}},
    BayesianPayoffModelNode{FixedPayoffModelNode{0.04}}}};
  const auto snapshot = ModelPerformance{config}.snapshot();

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

TEST(ModelPerformanceTest, RejectsInvalidHistoryAndPriors)
{
  EXPECT_THROW((ModelPerformanceConfig{ModelPerformanceHistoryPolicy{
                ModelPerformanceHistoryMode::RollingWindow, 0, 0.99}}),
               std::invalid_argument);
  EXPECT_THROW((ModelPerformanceBayesianConfig{
                BayesianWinModelNode{BetaBernoulliModelNode{1.0, 2.0}},
                BayesianPayoffModelNode{GammaPayoffModelNode{}},
                BayesianPayoffModelNode{GammaPayoffModelNode{}}}),
               std::invalid_argument);
  EXPECT_THROW((ModelPerformanceBayesianConfig{
                BayesianWinModelNode{BetaBernoulliModelNode{}},
                BayesianPayoffModelNode{GammaPayoffModelNode{0.0, 0.01, 1.0}},
                BayesianPayoffModelNode{GammaPayoffModelNode{}}}),
               std::invalid_argument);
  EXPECT_THROW(
   (ModelPerformanceBayesianConfig{
    BayesianWinModelNode{BetaBernoulliModelNode{}},
    BayesianPayoffModelNode{GammaPayoffModelNode{}},
    BayesianPayoffModelNode{GammaPayoffModelNode{0.01, 0.01, 0.0}}}),
   std::invalid_argument);
}
