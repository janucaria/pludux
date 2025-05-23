#include <utility>

#include <pludux/backtest/strategy.hpp>

namespace pludux::backtest {

Strategy::Strategy(double capital_risk,
                   screener::ScreenerFilter entry_filter,
                   screener::ScreenerFilter exit_filter,
                   backtest::StopLoss stop_loss,
                   backtest::TakeProfit take_profit)
: capital_risk_{capital_risk}
, entry_filter_{std::move(entry_filter)}
, exit_filter_{std::move(exit_filter)}
, stop_loss_{std::move(stop_loss)}
, take_profit_{std::move(take_profit)}
{
}

auto Strategy::capital_risk() const noexcept -> double
{
  return capital_risk_;
}

auto Strategy::entry_filter() const noexcept -> const screener::ScreenerFilter&
{
  return entry_filter_;
}

auto Strategy::exit_filter() const noexcept -> const screener::ScreenerFilter&
{
  return exit_filter_;
}

auto Strategy::stop_loss() const noexcept -> const backtest::StopLoss&
{
  return stop_loss_;
}

auto Strategy::take_profit() const noexcept -> const backtest::TakeProfit&
{
  return take_profit_;
}

} // namespace pludux::backtest
