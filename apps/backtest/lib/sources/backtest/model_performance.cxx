module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <limits>
#include <stdexcept>
#include <utility>

export module pludux.backtest:model_performance;

import :model_closed_position;
import :model_performance_model;

export namespace pludux::backtest {

enum class ModelPerformanceHistoryMode {
  All,
  RollingWindow,
  ExponentialDecay
};

enum class ModelPerformanceBreakEvenTreatment {
  Skip,
  CountAsWin,
  CountAsLoss
};

class ModelPerformanceHistoryPolicy {
public:
  ModelPerformanceHistoryPolicy(
   ModelPerformanceHistoryMode mode = ModelPerformanceHistoryMode::All,
   std::size_t rolling_window = 100,
   double exponential_decay = 0.99) noexcept
  : mode_{mode}
  , rolling_window_{rolling_window}
  , exponential_decay_{exponential_decay}
  {
  }

  auto operator==(const ModelPerformanceHistoryPolicy&) const noexcept
   -> bool = default;

  auto mode(this const ModelPerformanceHistoryPolicy& self) noexcept
   -> ModelPerformanceHistoryMode
  {
    return self.mode_;
  }

  auto
  rolling_window(this const ModelPerformanceHistoryPolicy& self) noexcept
   -> std::size_t
  {
    return self.rolling_window_;
  }

  auto
  exponential_decay(this const ModelPerformanceHistoryPolicy& self) noexcept
   -> double
  {
    return self.exponential_decay_;
  }

  void validate(this const ModelPerformanceHistoryPolicy& self)
  {
    if(self.rolling_window_ == 0) {
      throw std::invalid_argument{"Rolling history window must be positive"};
    }
    if(!std::isfinite(self.exponential_decay_) ||
       self.exponential_decay_ <= 0.0 || self.exponential_decay_ > 1.0) {
      throw std::invalid_argument{
       "Exponential history decay must be in (0, 1]"};
    }
  }

private:
  ModelPerformanceHistoryMode mode_{ModelPerformanceHistoryMode::All};
  std::size_t rolling_window_{100};
  double exponential_decay_{0.99};
};

class ModelPerformanceBayesianConfig {
public:
  ModelPerformanceBayesianConfig(
   BayesianWinModelNode win_probability_model =
    BayesianWinModelNode{BetaBernoulliModelNode{}},
   BayesianPayoffModelNode winning_payoff_model =
    BayesianPayoffModelNode{GammaPayoffModelNode{}},
   BayesianPayoffModelNode losing_payoff_model =
    BayesianPayoffModelNode{GammaPayoffModelNode{}})
  : win_probability_model_{std::move(win_probability_model)}
  , winning_payoff_model_{std::move(winning_payoff_model)}
  , losing_payoff_model_{std::move(losing_payoff_model)}
  {
    validate();
  }

  auto operator==(const ModelPerformanceBayesianConfig&) const noexcept
   -> bool = default;

  auto win_probability_model(
   this const ModelPerformanceBayesianConfig& self) noexcept
   -> const BayesianWinModelNode&
  {
    return self.win_probability_model_;
  }

  void win_probability_model(this ModelPerformanceBayesianConfig& self,
                             BayesianWinModelNode model)
  {
    static_cast<void>(node_to_model_method(model));
    self.win_probability_model_ = std::move(model);
  }

  auto winning_payoff_model(
   this const ModelPerformanceBayesianConfig& self) noexcept
   -> const BayesianPayoffModelNode&
  {
    return self.winning_payoff_model_;
  }

  void winning_payoff_model(this ModelPerformanceBayesianConfig& self,
                            BayesianPayoffModelNode model)
  {
    static_cast<void>(node_to_model_method(model));
    self.winning_payoff_model_ = std::move(model);
  }

  auto losing_payoff_model(
   this const ModelPerformanceBayesianConfig& self) noexcept
   -> const BayesianPayoffModelNode&
  {
    return self.losing_payoff_model_;
  }

  void losing_payoff_model(this ModelPerformanceBayesianConfig& self,
                           BayesianPayoffModelNode model)
  {
    static_cast<void>(node_to_model_method(model));
    self.losing_payoff_model_ = std::move(model);
  }

  void validate(this const ModelPerformanceBayesianConfig& self)
  {
    static_cast<void>(node_to_model_method(self.win_probability_model_));
    static_cast<void>(node_to_model_method(self.winning_payoff_model_));
    static_cast<void>(node_to_model_method(self.losing_payoff_model_));
  }

private:
  BayesianWinModelNode win_probability_model_{BetaBernoulliModelNode{}};
  BayesianPayoffModelNode winning_payoff_model_{GammaPayoffModelNode{}};
  BayesianPayoffModelNode losing_payoff_model_{GammaPayoffModelNode{}};
};

class ModelPerformanceConfig {
public:
  ModelPerformanceConfig(
   ModelPerformanceHistoryPolicy history = {},
   ModelPerformanceBayesianConfig bayesian = {},
   ModelPerformanceBreakEvenTreatment break_even_treatment =
     ModelPerformanceBreakEvenTreatment::CountAsLoss)
  : history_{std::move(history)}
  , bayesian_{std::move(bayesian)}
  , break_even_treatment_{break_even_treatment}
  {
    validate();
  }

  auto operator==(const ModelPerformanceConfig&) const noexcept
   -> bool = default;

  auto history(this const ModelPerformanceConfig& self) noexcept
   -> const ModelPerformanceHistoryPolicy&
  {
    return self.history_;
  }

  void history(this ModelPerformanceConfig& self,
                ModelPerformanceHistoryPolicy history)
  {
    history.validate();
    self.history_ = std::move(history);
  }

  auto bayesian(this const ModelPerformanceConfig& self) noexcept
   -> const ModelPerformanceBayesianConfig&
  {
    return self.bayesian_;
  }

  void bayesian(this ModelPerformanceConfig& self,
                 ModelPerformanceBayesianConfig bayesian)
  {
    bayesian.validate();
    self.bayesian_ = std::move(bayesian);
  }

  auto break_even_treatment(this const ModelPerformanceConfig& self) noexcept
   -> ModelPerformanceBreakEvenTreatment
  {
    return self.break_even_treatment_;
  }

  void break_even_treatment(
    this ModelPerformanceConfig& self,
    ModelPerformanceBreakEvenTreatment break_even_treatment)
  {
    validate_break_even_treatment(break_even_treatment);
    self.break_even_treatment_ = break_even_treatment;
  }

  void validate(this const ModelPerformanceConfig& self)
  {
    self.history_.validate();
    self.bayesian_.validate();
    validate_break_even_treatment(self.break_even_treatment_);
  }

private:
  ModelPerformanceHistoryPolicy history_{};
  ModelPerformanceBayesianConfig bayesian_{};
  ModelPerformanceBreakEvenTreatment break_even_treatment_{
   ModelPerformanceBreakEvenTreatment::CountAsLoss};

  static void validate_break_even_treatment(
   ModelPerformanceBreakEvenTreatment break_even_treatment)
  {
    switch(break_even_treatment) {
    case ModelPerformanceBreakEvenTreatment::Skip:
    case ModelPerformanceBreakEvenTreatment::CountAsWin:
    case ModelPerformanceBreakEvenTreatment::CountAsLoss:
      return;
    }
    throw std::invalid_argument{"Invalid break-even treatment"};
  }
};

class ModelPerformanceSnapshot {
public:
  ModelPerformanceSnapshot() = default;

  ModelPerformanceSnapshot(
   std::size_t lifetime_count,
   std::size_t current_winning_streak,
   std::size_t current_losing_streak,
   std::size_t maximum_winning_streak,
   std::size_t maximum_losing_streak,
   double effective_count,
   double win_rate,
   double break_even_rate,
   double loss_rate,
   double mean_return,
   double return_standard_deviation,
   BayesianWinSnapshot win_probability_posterior,
   BayesianPayoffSnapshot winning_payoff_posterior,
   BayesianPayoffSnapshot losing_payoff_posterior) noexcept
  : lifetime_count_{lifetime_count}
  , current_winning_streak_{current_winning_streak}
  , current_losing_streak_{current_losing_streak}
  , maximum_winning_streak_{maximum_winning_streak}
  , maximum_losing_streak_{maximum_losing_streak}
  , effective_count_{effective_count}
  , win_rate_{win_rate}
  , break_even_rate_{break_even_rate}
  , loss_rate_{loss_rate}
  , mean_return_{mean_return}
  , return_standard_deviation_{return_standard_deviation}
  , win_probability_posterior_{std::move(win_probability_posterior)}
  , winning_payoff_posterior_{std::move(winning_payoff_posterior)}
  , losing_payoff_posterior_{std::move(losing_payoff_posterior)}
  {
  }

  auto operator==(const ModelPerformanceSnapshot&) const noexcept
   -> bool = default;

  auto lifetime_count(this const ModelPerformanceSnapshot& self) noexcept
   -> std::size_t
  {
    return self.lifetime_count_;
  }

  auto
   current_winning_streak(this const ModelPerformanceSnapshot& self) noexcept
   -> std::size_t
  {
    return self.current_winning_streak_;
  }

  auto
   current_losing_streak(this const ModelPerformanceSnapshot& self) noexcept
   -> std::size_t
  {
    return self.current_losing_streak_;
  }

  auto
   maximum_winning_streak(this const ModelPerformanceSnapshot& self) noexcept
   -> std::size_t
  {
    return self.maximum_winning_streak_;
  }

  auto
   maximum_losing_streak(this const ModelPerformanceSnapshot& self) noexcept
   -> std::size_t
  {
    return self.maximum_losing_streak_;
  }

  auto effective_count(this const ModelPerformanceSnapshot& self) noexcept
   -> double
  {
    return self.effective_count_;
  }

  auto win_rate(this const ModelPerformanceSnapshot& self) noexcept -> double
  {
    return self.win_rate_;
  }

  auto break_even_rate(this const ModelPerformanceSnapshot& self) noexcept
   -> double
  {
    return self.break_even_rate_;
  }

  auto loss_rate(this const ModelPerformanceSnapshot& self) noexcept
   -> double
  {
    return self.loss_rate_;
  }

  auto mean_return(this const ModelPerformanceSnapshot& self) noexcept
   -> double
  {
    return self.mean_return_;
  }

  auto return_standard_deviation(
   this const ModelPerformanceSnapshot& self) noexcept -> double
  {
    return self.return_standard_deviation_;
  }

  auto win_probability_posterior(
   this const ModelPerformanceSnapshot& self) noexcept
   -> const BayesianWinSnapshot&
  {
    return self.win_probability_posterior_;
  }

  auto winning_payoff_posterior(
   this const ModelPerformanceSnapshot& self) noexcept
   -> const BayesianPayoffSnapshot&
  {
    return self.winning_payoff_posterior_;
  }

  auto
   losing_payoff_posterior(this const ModelPerformanceSnapshot& self) noexcept
   -> const BayesianPayoffSnapshot&
  {
    return self.losing_payoff_posterior_;
  }

private:
  std::size_t lifetime_count_{};
  std::size_t current_winning_streak_{};
  std::size_t current_losing_streak_{};
  std::size_t maximum_winning_streak_{};
  std::size_t maximum_losing_streak_{};
  double effective_count_{};
  double win_rate_{};
  double break_even_rate_{};
  double loss_rate_{};
  double mean_return_{};
  double return_standard_deviation_{};
  BayesianWinSnapshot win_probability_posterior_{};
  BayesianPayoffSnapshot winning_payoff_posterior_{};
  BayesianPayoffSnapshot losing_payoff_posterior_{};
};

class ModelPerformance {
public:
  explicit ModelPerformance(ModelPerformanceConfig config = {})
  : config_{std::move(config)}
  , win_probability_model_{node_to_model_method(
     config_.bayesian().win_probability_model())}
  , winning_payoff_model_{node_to_model_method(
     config_.bayesian().winning_payoff_model())}
  , losing_payoff_model_{
     node_to_model_method(config_.bayesian().losing_payoff_model())}
  {
  }

  auto config(this const ModelPerformance& self) noexcept
   -> const ModelPerformanceConfig&
  {
    return self.config_;
  }

  void observe(this ModelPerformance& self,
                const ModelClosedPosition& position)
  {
    const auto return_ratio = position.return_ratio();
    if(!std::isfinite(return_ratio)) {
      throw std::invalid_argument{"Strategy return must be finite"};
    }

    ++self.lifetime_count_;
    self.update_streaks(return_ratio);
    switch(self.config_.history().mode()) {
    case ModelPerformanceHistoryMode::All:
      self.add(return_ratio);
      break;
    case ModelPerformanceHistoryMode::RollingWindow:
      self.observations_.push_back(return_ratio);
      while(self.observations_.size() >
            self.config_.history().rolling_window()) {
        self.observations_.pop_front();
      }
      self.rebuild();
      break;
    case ModelPerformanceHistoryMode::ExponentialDecay:
      self.decay(self.config_.history().exponential_decay());
      self.add(return_ratio);
      break;
    }
  }

  auto snapshot(this const ModelPerformance& self)
   -> ModelPerformanceSnapshot
  {
    const auto outcome_weight = self.outcome_evidence_.win_weight +
                                self.outcome_evidence_.break_even_weight +
                                self.outcome_evidence_.loss_weight;
    const auto win_rate = outcome_weight > 0.0
                           ? self.outcome_evidence_.win_weight / outcome_weight
                           : 0.0;
    const auto break_even_rate =
     outcome_weight > 0.0
      ? self.outcome_evidence_.break_even_weight / outcome_weight
      : 0.0;
    const auto loss_rate =
     outcome_weight > 0.0 ? self.outcome_evidence_.loss_weight / outcome_weight
                          : 0.0;
    const auto standard_deviation =
     self.return_evidence_.effective_count > 1.0
      ? std::sqrt(self.return_evidence_.squared_deviation_sum /
                  (self.return_evidence_.effective_count - 1.0))
      : 0.0;

    return ModelPerformanceSnapshot{
     self.lifetime_count_,
     self.current_winning_streak_,
     self.current_losing_streak_,
     self.maximum_winning_streak_,
     self.maximum_losing_streak_,
     self.return_evidence_.effective_count,
     win_rate,
     break_even_rate,
     loss_rate,
     self.return_evidence_.mean,
     standard_deviation,
     self.win_probability_model_.evaluate(self.binary_evidence_),
     self.winning_payoff_model_.evaluate(self.winning_payoff_evidence_),
     self.losing_payoff_model_.evaluate(self.losing_payoff_evidence_)};
  }

private:
  struct WeightedReturnEvidence {
    double effective_count{};
    double mean{};
    double squared_deviation_sum{};
  };

  struct WeightedOutcomeEvidence {
    double win_weight{};
    double break_even_weight{};
    double loss_weight{};
  };

  ModelPerformanceConfig config_;
  BayesianWinModelMethod win_probability_model_;
  BayesianPayoffModelMethod winning_payoff_model_;
  BayesianPayoffModelMethod losing_payoff_model_;
  std::size_t lifetime_count_{};
  std::size_t current_winning_streak_{};
  std::size_t current_losing_streak_{};
  std::size_t maximum_winning_streak_{};
  std::size_t maximum_losing_streak_{};
  std::deque<double> observations_{};
  WeightedReturnEvidence return_evidence_{};
  WeightedOutcomeEvidence outcome_evidence_{};
  WeightedBinaryEvidence binary_evidence_{};
  WeightedPayoffEvidence winning_payoff_evidence_{};
  WeightedPayoffEvidence losing_payoff_evidence_{};

  void update_streaks(this ModelPerformance& self, double value) noexcept
  {
    if(value > 0.0) {
      ++self.current_winning_streak_;
      self.current_losing_streak_ = 0;
      self.maximum_winning_streak_ =
       std::max(self.maximum_winning_streak_, self.current_winning_streak_);
    } else if(value < 0.0) {
      self.current_winning_streak_ = 0;
      ++self.current_losing_streak_;
      self.maximum_losing_streak_ =
       std::max(self.maximum_losing_streak_, self.current_losing_streak_);
    } else {
      self.current_winning_streak_ = 0;
      self.current_losing_streak_ = 0;
    }
  }

  void add(this ModelPerformance& self, double value) noexcept
  {
    const auto previous_weight = self.return_evidence_.effective_count;
    const auto new_weight = previous_weight + 1.0;
    const auto delta = value - self.return_evidence_.mean;
    self.return_evidence_.mean += delta / new_weight;
    self.return_evidence_.squared_deviation_sum +=
     delta * (value - self.return_evidence_.mean);
    self.return_evidence_.effective_count = new_weight;

    if(value > 0.0) {
      self.outcome_evidence_.win_weight += 1.0;
      self.binary_evidence_.win_weight += 1.0;
      self.winning_payoff_evidence_.effective_count += 1.0;
      self.winning_payoff_evidence_.sum += value;
    } else if(value < 0.0) {
      self.outcome_evidence_.loss_weight += 1.0;
      self.binary_evidence_.loss_weight += 1.0;
      self.losing_payoff_evidence_.effective_count += 1.0;
      self.losing_payoff_evidence_.sum -= value;
    } else {
      self.outcome_evidence_.break_even_weight += 1.0;
      switch(self.config_.break_even_treatment()) {
      case ModelPerformanceBreakEvenTreatment::Skip:
        break;
      case ModelPerformanceBreakEvenTreatment::CountAsWin:
        self.binary_evidence_.win_weight += 1.0;
        break;
      case ModelPerformanceBreakEvenTreatment::CountAsLoss:
        self.binary_evidence_.loss_weight += 1.0;
        break;
      }
    }
  }

  void decay(this ModelPerformance& self, double factor) noexcept
  {
    self.return_evidence_.effective_count *= factor;
    self.return_evidence_.squared_deviation_sum *= factor;
    self.outcome_evidence_.win_weight *= factor;
    self.outcome_evidence_.break_even_weight *= factor;
    self.outcome_evidence_.loss_weight *= factor;
    self.binary_evidence_.win_weight *= factor;
    self.binary_evidence_.loss_weight *= factor;
    self.winning_payoff_evidence_.effective_count *= factor;
    self.winning_payoff_evidence_.sum *= factor;
    self.losing_payoff_evidence_.effective_count *= factor;
    self.losing_payoff_evidence_.sum *= factor;

    if(self.return_evidence_.effective_count <=
       std::numeric_limits<double>::epsilon()) {
      self.return_evidence_ = {};
      self.outcome_evidence_ = {};
      self.binary_evidence_ = {};
      self.winning_payoff_evidence_ = {};
      self.losing_payoff_evidence_ = {};
    }
  }

  void rebuild(this ModelPerformance& self) noexcept
  {
    self.return_evidence_ = {};
    self.outcome_evidence_ = {};
    self.binary_evidence_ = {};
    self.winning_payoff_evidence_ = {};
    self.losing_payoff_evidence_ = {};
    for(const auto value : self.observations_) {
      self.add(value);
    }
  }
};

} // namespace pludux::backtest
