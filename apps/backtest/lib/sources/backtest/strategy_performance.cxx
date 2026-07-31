module;

#include <cmath>
#include <cstddef>
#include <deque>
#include <limits>
#include <stdexcept>
#include <utility>

export module pludux.backtest:strategy_performance;

import :strategy_closed_position;
import :strategy_performance_model;

export namespace pludux::backtest {

enum class StrategyPerformanceHistoryMode {
  All,
  RollingWindow,
  ExponentialDecay
};

class StrategyPerformanceHistoryPolicy {
public:
  StrategyPerformanceHistoryPolicy(
   StrategyPerformanceHistoryMode mode = StrategyPerformanceHistoryMode::All,
   std::size_t rolling_window = 100,
   double exponential_decay = 0.99) noexcept
  : mode_{mode}
  , rolling_window_{rolling_window}
  , exponential_decay_{exponential_decay}
  {
  }

  auto operator==(const StrategyPerformanceHistoryPolicy&) const noexcept
   -> bool = default;

  auto mode(this const StrategyPerformanceHistoryPolicy& self) noexcept
   -> StrategyPerformanceHistoryMode
  {
    return self.mode_;
  }

  auto
  rolling_window(this const StrategyPerformanceHistoryPolicy& self) noexcept
   -> std::size_t
  {
    return self.rolling_window_;
  }

  auto
  exponential_decay(this const StrategyPerformanceHistoryPolicy& self) noexcept
   -> double
  {
    return self.exponential_decay_;
  }

  void validate(this const StrategyPerformanceHistoryPolicy& self)
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
  StrategyPerformanceHistoryMode mode_{StrategyPerformanceHistoryMode::All};
  std::size_t rolling_window_{100};
  double exponential_decay_{0.99};
};

class StrategyPerformanceBayesianConfig {
public:
  StrategyPerformanceBayesianConfig(
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

  auto operator==(const StrategyPerformanceBayesianConfig&) const noexcept
   -> bool = default;

  auto win_probability_model(
   this const StrategyPerformanceBayesianConfig& self) noexcept
   -> const BayesianWinModelNode&
  {
    return self.win_probability_model_;
  }

  void win_probability_model(this StrategyPerformanceBayesianConfig& self,
                             BayesianWinModelNode model)
  {
    static_cast<void>(node_to_model_method(model));
    self.win_probability_model_ = std::move(model);
  }

  auto winning_payoff_model(
   this const StrategyPerformanceBayesianConfig& self) noexcept
   -> const BayesianPayoffModelNode&
  {
    return self.winning_payoff_model_;
  }

  void winning_payoff_model(this StrategyPerformanceBayesianConfig& self,
                            BayesianPayoffModelNode model)
  {
    static_cast<void>(node_to_model_method(model));
    self.winning_payoff_model_ = std::move(model);
  }

  auto losing_payoff_model(
   this const StrategyPerformanceBayesianConfig& self) noexcept
   -> const BayesianPayoffModelNode&
  {
    return self.losing_payoff_model_;
  }

  void losing_payoff_model(this StrategyPerformanceBayesianConfig& self,
                           BayesianPayoffModelNode model)
  {
    static_cast<void>(node_to_model_method(model));
    self.losing_payoff_model_ = std::move(model);
  }

  void validate(this const StrategyPerformanceBayesianConfig& self)
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

class StrategyPerformanceConfig {
public:
  StrategyPerformanceConfig(StrategyPerformanceHistoryPolicy history = {},
                            StrategyPerformanceBayesianConfig bayesian = {})
  : history_{std::move(history)}
  , bayesian_{std::move(bayesian)}
  {
    validate();
  }

  auto operator==(const StrategyPerformanceConfig&) const noexcept
   -> bool = default;

  auto history(this const StrategyPerformanceConfig& self) noexcept
   -> const StrategyPerformanceHistoryPolicy&
  {
    return self.history_;
  }

  void history(this StrategyPerformanceConfig& self,
               StrategyPerformanceHistoryPolicy history)
  {
    history.validate();
    self.history_ = std::move(history);
  }

  auto bayesian(this const StrategyPerformanceConfig& self) noexcept
   -> const StrategyPerformanceBayesianConfig&
  {
    return self.bayesian_;
  }

  void bayesian(this StrategyPerformanceConfig& self,
                StrategyPerformanceBayesianConfig bayesian)
  {
    bayesian.validate();
    self.bayesian_ = std::move(bayesian);
  }

  void validate(this const StrategyPerformanceConfig& self)
  {
    self.history_.validate();
    self.bayesian_.validate();
  }

private:
  StrategyPerformanceHistoryPolicy history_{};
  StrategyPerformanceBayesianConfig bayesian_{};
};

class StrategyPerformanceSnapshot {
public:
  StrategyPerformanceSnapshot() = default;

  StrategyPerformanceSnapshot(
   std::size_t lifetime_count,
   double effective_count,
   double win_rate,
   double mean_return,
   double return_standard_deviation,
   BayesianWinSnapshot win_probability_posterior,
   BayesianPayoffSnapshot winning_payoff_posterior,
   BayesianPayoffSnapshot losing_payoff_posterior) noexcept
  : lifetime_count_{lifetime_count}
  , effective_count_{effective_count}
  , win_rate_{win_rate}
  , mean_return_{mean_return}
  , return_standard_deviation_{return_standard_deviation}
  , win_probability_posterior_{std::move(win_probability_posterior)}
  , winning_payoff_posterior_{std::move(winning_payoff_posterior)}
  , losing_payoff_posterior_{std::move(losing_payoff_posterior)}
  {
  }

  auto operator==(const StrategyPerformanceSnapshot&) const noexcept
   -> bool = default;

  auto lifetime_count(this const StrategyPerformanceSnapshot& self) noexcept
   -> std::size_t
  {
    return self.lifetime_count_;
  }

  auto effective_count(this const StrategyPerformanceSnapshot& self) noexcept
   -> double
  {
    return self.effective_count_;
  }

  auto win_rate(this const StrategyPerformanceSnapshot& self) noexcept -> double
  {
    return self.win_rate_;
  }

  auto mean_return(this const StrategyPerformanceSnapshot& self) noexcept
   -> double
  {
    return self.mean_return_;
  }

  auto return_standard_deviation(
   this const StrategyPerformanceSnapshot& self) noexcept -> double
  {
    return self.return_standard_deviation_;
  }

  auto win_probability_posterior(
   this const StrategyPerformanceSnapshot& self) noexcept
   -> const BayesianWinSnapshot&
  {
    return self.win_probability_posterior_;
  }

  auto winning_payoff_posterior(
   this const StrategyPerformanceSnapshot& self) noexcept
   -> const BayesianPayoffSnapshot&
  {
    return self.winning_payoff_posterior_;
  }

  auto
  losing_payoff_posterior(this const StrategyPerformanceSnapshot& self) noexcept
   -> const BayesianPayoffSnapshot&
  {
    return self.losing_payoff_posterior_;
  }

private:
  std::size_t lifetime_count_{};
  double effective_count_{};
  double win_rate_{};
  double mean_return_{};
  double return_standard_deviation_{};
  BayesianWinSnapshot win_probability_posterior_{};
  BayesianPayoffSnapshot winning_payoff_posterior_{};
  BayesianPayoffSnapshot losing_payoff_posterior_{};
};

class StrategyPerformance {
public:
  explicit StrategyPerformance(StrategyPerformanceConfig config = {})
  : config_{std::move(config)}
  , win_probability_model_{node_to_model_method(
     config_.bayesian().win_probability_model())}
  , winning_payoff_model_{node_to_model_method(
     config_.bayesian().winning_payoff_model())}
  , losing_payoff_model_{
     node_to_model_method(config_.bayesian().losing_payoff_model())}
  {
  }

  auto config(this const StrategyPerformance& self) noexcept
   -> const StrategyPerformanceConfig&
  {
    return self.config_;
  }

  void observe(this StrategyPerformance& self,
               const StrategyClosedPosition& position)
  {
    const auto return_ratio = position.return_ratio();
    if(!std::isfinite(return_ratio)) {
      throw std::invalid_argument{"Strategy return must be finite"};
    }

    ++self.lifetime_count_;
    switch(self.config_.history().mode()) {
    case StrategyPerformanceHistoryMode::All:
      self.add(return_ratio);
      break;
    case StrategyPerformanceHistoryMode::RollingWindow:
      self.observations_.push_back(return_ratio);
      while(self.observations_.size() >
            self.config_.history().rolling_window()) {
        self.observations_.pop_front();
      }
      self.rebuild();
      break;
    case StrategyPerformanceHistoryMode::ExponentialDecay:
      self.decay(self.config_.history().exponential_decay());
      self.add(return_ratio);
      break;
    }
  }

  auto snapshot(this const StrategyPerformance& self)
   -> StrategyPerformanceSnapshot
  {
    const auto resolved_weight =
     self.binary_evidence_.win_weight + self.binary_evidence_.loss_weight;
    const auto win_rate = resolved_weight > 0.0
                           ? self.binary_evidence_.win_weight / resolved_weight
                           : 0.0;
    const auto standard_deviation =
     self.return_evidence_.effective_count > 1.0
      ? std::sqrt(self.return_evidence_.squared_deviation_sum /
                  (self.return_evidence_.effective_count - 1.0))
      : 0.0;

    return StrategyPerformanceSnapshot{
     self.lifetime_count_,
     self.return_evidence_.effective_count,
     win_rate,
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

  StrategyPerformanceConfig config_;
  BayesianWinModelMethod win_probability_model_;
  BayesianPayoffModelMethod winning_payoff_model_;
  BayesianPayoffModelMethod losing_payoff_model_;
  std::size_t lifetime_count_{};
  std::deque<double> observations_{};
  WeightedReturnEvidence return_evidence_{};
  WeightedBinaryEvidence binary_evidence_{};
  WeightedPayoffEvidence winning_payoff_evidence_{};
  WeightedPayoffEvidence losing_payoff_evidence_{};

  void add(this StrategyPerformance& self, double value) noexcept
  {
    const auto previous_weight = self.return_evidence_.effective_count;
    const auto new_weight = previous_weight + 1.0;
    const auto delta = value - self.return_evidence_.mean;
    self.return_evidence_.mean += delta / new_weight;
    self.return_evidence_.squared_deviation_sum +=
     delta * (value - self.return_evidence_.mean);
    self.return_evidence_.effective_count = new_weight;

    if(value > 0.0) {
      self.binary_evidence_.win_weight += 1.0;
      self.winning_payoff_evidence_.effective_count += 1.0;
      self.winning_payoff_evidence_.sum += value;
    } else if(value < 0.0) {
      self.binary_evidence_.loss_weight += 1.0;
      self.losing_payoff_evidence_.effective_count += 1.0;
      self.losing_payoff_evidence_.sum -= value;
    }
  }

  void decay(this StrategyPerformance& self, double factor) noexcept
  {
    self.return_evidence_.effective_count *= factor;
    self.return_evidence_.squared_deviation_sum *= factor;
    self.binary_evidence_.win_weight *= factor;
    self.binary_evidence_.loss_weight *= factor;
    self.winning_payoff_evidence_.effective_count *= factor;
    self.winning_payoff_evidence_.sum *= factor;
    self.losing_payoff_evidence_.effective_count *= factor;
    self.losing_payoff_evidence_.sum *= factor;

    if(self.return_evidence_.effective_count <=
       std::numeric_limits<double>::epsilon()) {
      self.return_evidence_ = {};
      self.binary_evidence_ = {};
      self.winning_payoff_evidence_ = {};
      self.losing_payoff_evidence_ = {};
    }
  }

  void rebuild(this StrategyPerformance& self) noexcept
  {
    self.return_evidence_ = {};
    self.binary_evidence_ = {};
    self.winning_payoff_evidence_ = {};
    self.losing_payoff_evidence_ = {};
    for(const auto value : self.observations_) {
      self.add(value);
    }
  }
};

} // namespace pludux::backtest
