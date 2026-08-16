module;

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

export module pludux.backtest:entry_filter_method_context;

import pludux;

import :backtest_method_context;
import :requested_order;
import :model_performance;

export namespace pludux::backtest {

class EntryFilterMethodContext {
public:
  EntryFilterMethodContext(const BacktestAccountState& account,
                            const ModelPerformanceSnapshot& performance,
                           const RequestedOrder& requested_order) noexcept
  : account_{account}
  , performance_{performance}
  , requested_order_{requested_order}
  {
  }

  auto call_series_method(this const EntryFilterMethodContext&,
                          const std::string&,
                          AssetSnapshot) noexcept -> double
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto call_series_method(this const EntryFilterMethodContext&,
                          const std::string&,
                          AssetSnapshot,
                          MethodOutput) noexcept -> double
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto get_series_result(this const EntryFilterMethodContext&,
                         const std::string&,
                         std::size_t) noexcept -> double
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto get_series_results(this EntryFilterMethodContext& self,
                          const auto&) noexcept -> std::vector<double>&
  {
    return self.unused_series_results_;
  }

  auto index(this const EntryFilterMethodContext&) noexcept -> std::size_t
  {
    return 0;
  }

  auto equity(this const EntryFilterMethodContext& self) noexcept -> double
  {
    return self.account_.equity();
  }

  auto equity_percent(this const EntryFilterMethodContext& self) noexcept
   -> double
  {
    return self.account_.equity_percent();
  }

  auto drawdown(this const EntryFilterMethodContext& self) noexcept -> double
  {
    return self.account_.drawdown();
  }

  auto performance(this const EntryFilterMethodContext& self) noexcept
   -> const ModelPerformanceSnapshot&
  {
    return self.performance_;
  }

  auto requested_order(this const EntryFilterMethodContext& self) noexcept
   -> const RequestedOrder&
  {
    return self.requested_order_;
  }

private:
  const BacktestAccountState& account_;
  const ModelPerformanceSnapshot& performance_;
  const RequestedOrder& requested_order_;
  std::vector<double> unused_series_results_;
};

static_assert(MethodContextable<EntryFilterMethodContext>);

} // namespace pludux::backtest
