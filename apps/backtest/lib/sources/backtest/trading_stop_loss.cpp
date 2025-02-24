#include <numeric>

#include <pludux/backtest/trading_stop_loss.hpp>

namespace pludux::backtest {

TradingStopLoss::TradingStopLoss(bool is_disabled,
                                 bool is_trailing,
                                 double risk,
                                 double stop_price)
: is_disabled_{is_disabled}
, is_trailing_{is_trailing}
, risk_{risk}
, stop_price_{stop_price}
{
}

auto TradingStopLoss::exit_price() const noexcept -> double
{
  return !is_disabled_ ? stop_price_ : std::numeric_limits<double>::quiet_NaN();
}

auto TradingStopLoss::operator()(const RunningState& running_state) -> bool
{
  if(is_trailing_) {
    const auto price = running_state.price();
    const auto new_stop_price = price - risk_;
    if(new_stop_price > stop_price_) {
      stop_price_ = new_stop_price;
    }
  }

  const auto signal_price = running_state.low();
  const auto reference_price = exit_price();
  return signal_price <= reference_price;
}

} // namespace pludux::backtest
