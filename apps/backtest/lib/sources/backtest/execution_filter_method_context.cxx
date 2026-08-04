module;

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:execution_filter_method_context;

import pludux;

import :backtest_method_context;
import :strategy_performance;

export namespace pludux::backtest {

class ExecutionFilterMethodContext {
public:
  ExecutionFilterMethodContext(
   BacktestMethodContext base_context,
   const StrategyPerformanceSnapshot& performance) noexcept
  : base_context_{std::move(base_context)}
  , performance_{performance}
  {
  }

  auto call_series_method(this const ExecutionFilterMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot) noexcept -> double
  {
    return self.base_context_.call_series_method(name,
                                                 std::move(asset_snapshot));
  }

  auto call_series_method(this const ExecutionFilterMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot,
                          MethodOutput output) noexcept -> double
  {
    return self.base_context_.call_series_method(
     name, std::move(asset_snapshot), output);
  }

  auto get_series_result(this const ExecutionFilterMethodContext& self,
                         const std::string& name,
                         std::size_t result_index) noexcept -> double
  {
    return self.base_context_.get_series_result(name, result_index);
  }

  auto get_series_results(this ExecutionFilterMethodContext& self,
                          const auto& method_key) noexcept
   -> std::vector<double>&
  {
    return self.base_context_.get_series_results(method_key);
  }

  auto index(this const ExecutionFilterMethodContext& self) noexcept
   -> std::size_t
  {
    return self.base_context_.index();
  }

  auto equity(this const ExecutionFilterMethodContext& self) noexcept -> double
  {
    return self.base_context_.equity();
  }

  auto equity_percent(this const ExecutionFilterMethodContext& self) noexcept
   -> double
  {
    return self.base_context_.equity_percent();
  }

  auto drawdown(this const ExecutionFilterMethodContext& self) noexcept
   -> double
  {
    return self.base_context_.drawdown();
  }

  auto performance(this const ExecutionFilterMethodContext& self) noexcept
   -> const StrategyPerformanceSnapshot&
  {
    return self.performance_;
  }

private:
  BacktestMethodContext base_context_;
  const StrategyPerformanceSnapshot& performance_;
};

static_assert(MethodContextable<ExecutionFilterMethodContext>);

} // namespace pludux::backtest
