module;

#include <cstddef>
#include <functional>
#include <string_view>

export module pludux.backtest:strategy_performance_method;

import pludux;

import :execution_filter_method_context;
import :strategy_performance;

export namespace pludux::backtest {

enum class StrategyPerformanceMetric {
  LifetimeCount,
  EffectiveCount,
  WinRate,
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

class StrategyPerformanceMethod {
public:
  explicit StrategyPerformanceMethod(StrategyPerformanceMetric metric) noexcept
  : metric_{metric}
  {
  }

  auto operator==(const StrategyPerformanceMethod&) const noexcept
   -> bool = default;

  auto metric(this const StrategyPerformanceMethod& self) noexcept
   -> StrategyPerformanceMetric
  {
    return self.metric_;
  }

private:
  StrategyPerformanceMetric metric_{StrategyPerformanceMetric::LifetimeCount};
};

auto hash_series_method(const StrategyPerformanceMethod& method) noexcept
 -> std::size_t
{
  auto result =
   std::hash<std::string_view>{}("pludux.backtest.StrategyPerformanceMethod");
  result ^= static_cast<std::size_t>(method.metric()) + 0x9e3779b9U +
            (result << 6U) + (result >> 2U);
  return result;
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const StrategyPerformanceMethod& method,
                       AssetSnapshot,
                       ExecutionFilterMethodContext context) noexcept -> double
{
  const auto& performance = context.performance();
  switch(method.metric()) {
  case StrategyPerformanceMetric::LifetimeCount:
    return static_cast<double>(performance.lifetime_count());
  case StrategyPerformanceMetric::EffectiveCount:
    return performance.effective_count();
  case StrategyPerformanceMetric::WinRate:
    return performance.win_rate();
  case StrategyPerformanceMetric::MeanReturn:
    return performance.mean_return();
  case StrategyPerformanceMetric::ReturnStandardDeviation:
    return performance.return_standard_deviation();
  case StrategyPerformanceMetric::BayesianWinProbability:
    return performance.win_probability_posterior().probability;
  case StrategyPerformanceMetric::BayesianWinProbabilityLower95:
    return performance.win_probability_posterior().lower_95;
  case StrategyPerformanceMetric::BayesianWinProbabilityUpper95:
    return performance.win_probability_posterior().upper_95;
  case StrategyPerformanceMetric::BayesianWinningPayoff:
    return performance.winning_payoff_posterior().mean;
  case StrategyPerformanceMetric::BayesianWinningPayoffLower95:
    return performance.winning_payoff_posterior().lower_95;
  case StrategyPerformanceMetric::BayesianWinningPayoffUpper95:
    return performance.winning_payoff_posterior().upper_95;
  case StrategyPerformanceMetric::BayesianLosingPayoff:
    return performance.losing_payoff_posterior().mean;
  case StrategyPerformanceMetric::BayesianLosingPayoffLower95:
    return performance.losing_payoff_posterior().lower_95;
  case StrategyPerformanceMetric::BayesianLosingPayoffUpper95:
    return performance.losing_payoff_posterior().upper_95;
  }
  return 0.0;
}

} // namespace pludux::backtest
