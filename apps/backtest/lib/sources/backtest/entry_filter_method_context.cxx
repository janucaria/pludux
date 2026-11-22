module;

#include <compare>

export module pludux.backtest:entry_filter_method_context;

import pludux;

import :backtest_method_context;
import :requested_order;
import :strategy_performance;

export namespace pludux::backtest {

class EntryFilterMethodContext {
public:
  EntryFilterMethodContext(const BacktestAccountState& account,
                            const StrategyPerformanceSnapshot& performance,
                           const RequestedOrder& requested_order) noexcept
  : account_{account}
  , performance_{performance}
  , requested_order_{requested_order}
  {
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
   -> const StrategyPerformanceSnapshot&
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
  const StrategyPerformanceSnapshot& performance_;
  const RequestedOrder& requested_order_;
};

using EntryFilterNode = ErasedNode<EntryFilterMethodContext>;
using EntryFilterMethod = ErasedSeriesMethod<EntryFilterMethodContext>;

} // namespace pludux::backtest
