module;

#include <cstddef>
#include <limits>
#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:backtest_method_context;

import pludux;

import :backtest_timeline;

export namespace pludux::backtest {

class BacktestMethodContext {
public:
  BacktestMethodContext(
   DefaultMethodContext default_context,
   const OrderedNamedRegistry<AnySeriesMethod>& series_methods,
   const BacktestTimeline& timeline) noexcept
  : default_context_{std::move(default_context)}
  , series_methods_{series_methods}
  , timeline_{timeline}
  {
  }

  auto call_series_method(this const BacktestMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot) noexcept -> double
  {
    if(const auto method_opt = self.series_methods_.get(name);
       method_opt.has_value()) {
      return evaluate_series_method(method_opt.value(), asset_snapshot, self);
    }
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto call_series_method(this const BacktestMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot,
                          MethodOutput output) noexcept -> double
  {
    if(const auto method_opt = self.series_methods_.get(name);
       method_opt.has_value()) {
      return evaluate_series_method(
       output, method_opt.value(), asset_snapshot, self);
    }
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto get_series_result(this const BacktestMethodContext& self,
                         const std::string& name,
                         std::size_t result_index) noexcept -> double
  {
    return self.default_context_.get_series_result(name, result_index);
  }

  auto get_series_results(this BacktestMethodContext& self,
                          const auto& method_key) noexcept
   -> std::vector<double>&
  {
    return self.default_context_.get_series_results(method_key);
  }

  auto index(this const BacktestMethodContext& self) noexcept -> std::size_t
  {
    return self.default_context_.index();
  }

  auto previous_equity(this const BacktestMethodContext& self) noexcept
   -> double
  {
    const auto index = self.index();
    if(index == 0 || self.timeline_.empty()) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const auto previous_index = index - 1;
    if(previous_index >= self.timeline_.size()) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    return self.timeline_.equity(previous_index);
  }

  auto previous_equity_percent(this const BacktestMethodContext& self) noexcept
   -> double
  {
    const auto index = self.index();
    if(index == 0 || self.timeline_.empty()) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const auto previous_index = index - 1;
    if(previous_index >= self.timeline_.size()) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const auto initial_capital = self.timeline_.initial_capital(previous_index);
    if(initial_capital == 0.0) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    return self.timeline_.equity(previous_index) / initial_capital * 100.0;
  }

  auto previous_drawdown(this const BacktestMethodContext& self) noexcept
   -> double
  {
    const auto index = self.index();
    if(index == 0 || self.timeline_.empty()) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const auto previous_index = index - 1;
    if(previous_index >= self.timeline_.size()) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    return self.timeline_.drawdown(previous_index);
  }

private:
  DefaultMethodContext default_context_;
  const OrderedNamedRegistry<AnySeriesMethod>& series_methods_;
  const BacktestTimeline& timeline_;
};

} // namespace pludux::backtest
