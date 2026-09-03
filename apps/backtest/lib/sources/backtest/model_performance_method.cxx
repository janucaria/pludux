module;

#include <cstddef>
#include <functional>
#include <string_view>

export module pludux.backtest:model_performance_method;

import pludux;

import :entry_filter_method_context;
import :model_performance;

export namespace pludux::backtest {

enum class ModelPerformanceMetric {
  LifetimeCount,
  EffectiveCount,
  WinRate,
  BreakEvenRate,
  LossRate,
  CurrentWinningStreak,
  CurrentLosingStreak,
  MaximumWinningStreak,
  MaximumLosingStreak,
  MeanReturn,
  ReturnStandardDeviation,
  BayesianWinProbability,
  BayesianWinProbabilityLower95,
  BayesianWinProbabilityUpper95,
  BayesianWinningPayoff,
  BayesianWinningPayoffLower95,
  BayesianWinningPayoffUpper95,
  BayesianLosingPayoff,
  BayesianLosingPayoffLower95,
  BayesianLosingPayoffUpper95
};

class ModelPerformanceMethod {
public:
  explicit ModelPerformanceMethod(ModelPerformanceMetric metric) noexcept
  : metric_{metric}
  {
  }

  auto operator==(const ModelPerformanceMethod&) const noexcept
   -> bool = default;

  auto metric(this const ModelPerformanceMethod& self) noexcept
   -> ModelPerformanceMetric
  {
    return self.metric_;
  }

private:
  ModelPerformanceMetric metric_{ModelPerformanceMetric::LifetimeCount};
};

auto hash_series_method(const ModelPerformanceMethod& method) noexcept
 -> std::size_t
{
  auto result =
   std::hash<std::string_view>{}("pludux.backtest.ModelPerformanceMethod");
  result ^= static_cast<std::size_t>(method.metric()) + 0x9e3779b9U +
            (result << 6U) + (result >> 2U);
  return result;
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                        const ModelPerformanceMethod& method,
                       AssetSnapshot,
                       EntryFilterMethodContext context) noexcept -> double
{
  const auto& performance = context.performance();
  switch(method.metric()) {
  case ModelPerformanceMetric::LifetimeCount:
    return static_cast<double>(performance.lifetime_count());
  case ModelPerformanceMetric::EffectiveCount:
    return performance.effective_count();
  case ModelPerformanceMetric::WinRate:
    return performance.win_rate();
  case ModelPerformanceMetric::BreakEvenRate:
    return performance.break_even_rate();
  case ModelPerformanceMetric::LossRate:
    return performance.loss_rate();
  case ModelPerformanceMetric::CurrentWinningStreak:
    return static_cast<double>(performance.current_winning_streak());
  case ModelPerformanceMetric::CurrentLosingStreak:
    return static_cast<double>(performance.current_losing_streak());
  case ModelPerformanceMetric::MaximumWinningStreak:
    return static_cast<double>(performance.maximum_winning_streak());
  case ModelPerformanceMetric::MaximumLosingStreak:
    return static_cast<double>(performance.maximum_losing_streak());
  case ModelPerformanceMetric::MeanReturn:
    return performance.mean_return();
  case ModelPerformanceMetric::ReturnStandardDeviation:
    return performance.return_standard_deviation();
  case ModelPerformanceMetric::BayesianWinProbability:
    return performance.win_probability_posterior().probability;
  case ModelPerformanceMetric::BayesianWinProbabilityLower95:
    return performance.win_probability_posterior().lower_95;
  case ModelPerformanceMetric::BayesianWinProbabilityUpper95:
    return performance.win_probability_posterior().upper_95;
  case ModelPerformanceMetric::BayesianWinningPayoff:
    return performance.winning_payoff_posterior().mean;
  case ModelPerformanceMetric::BayesianWinningPayoffLower95:
    return performance.winning_payoff_posterior().lower_95;
  case ModelPerformanceMetric::BayesianWinningPayoffUpper95:
    return performance.winning_payoff_posterior().upper_95;
  case ModelPerformanceMetric::BayesianLosingPayoff:
    return performance.losing_payoff_posterior().mean;
  case ModelPerformanceMetric::BayesianLosingPayoffLower95:
    return performance.losing_payoff_posterior().lower_95;
  case ModelPerformanceMetric::BayesianLosingPayoffUpper95:
    return performance.losing_payoff_posterior().upper_95;
  }
  return 0.0;
}

} // namespace pludux::backtest
