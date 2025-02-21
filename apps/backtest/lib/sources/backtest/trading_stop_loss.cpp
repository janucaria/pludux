#include <pludux/backtest/trading_stop_loss.hpp>

namespace pludux::backtest {

TradingStopLoss::TradingStopLoss(double stop_price,
                                 screener::ScreenerMethod signal_price_method)
: stop_price_{stop_price}
, signal_price_method_{std::move(signal_price_method)}
{
}

auto TradingStopLoss::exit_price() const noexcept -> double
{
  return stop_price_;
}

auto TradingStopLoss::operator()(const AssetSnapshot& asset) -> bool
{
  const auto signal_price = signal_price_method_(asset)[0];
  const auto reference_price = stop_price_;
  return signal_price <= stop_price_;
}

} // namespace pludux::backtest
