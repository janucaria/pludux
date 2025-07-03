#include <limits>
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
, initial_stop_price_{stop_price}
{
}

auto TradingStopLoss::is_trailing() const noexcept -> bool
{
  return is_trailing_;
}

auto TradingStopLoss::initial_exit_price() const noexcept -> double
{
  return !is_disabled_ ? initial_stop_price_
                       : std::numeric_limits<double>::quiet_NaN();
}

auto TradingStopLoss::exit_price() const noexcept -> double
{
  return !is_disabled_ ? stop_price_ : std::numeric_limits<double>::quiet_NaN();
}

auto TradingStopLoss::operator()(const AssetSnapshot& asset_snapshot) -> bool
{
  const auto stop_price = exit_price();
  const auto signal_price = asset_snapshot.get_low();
  const auto stop = signal_price <= stop_price;

  if(stop || !is_trailing_) {
    return stop;
  }

  const auto high = asset_snapshot.get_high();
  const auto new_stop_price = high - risk_;
  if(new_stop_price > stop_price_) {
    stop_price_ = new_stop_price;
  }

  return asset_snapshot.get_close() <= stop_price_;
}

} // namespace pludux::backtest
