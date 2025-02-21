#include <pludux/backtest/trading_take_profit.hpp>

namespace pludux::backtest {

TradingTakeProfit::TradingTakeProfit(
 double profit_price, screener::ScreenerMethod signal_price_method)
: profit_price_{profit_price}
, signal_price_method_{std::move(signal_price_method)}
{
}

auto TradingTakeProfit::exit_price() const noexcept -> double
{
  return profit_price_;
}

auto TradingTakeProfit::operator()(const AssetSnapshot& asset) -> bool
{
  const auto signal_price = signal_price_method_(asset)[0];
  const auto reference_price = profit_price_;
  return signal_price >= profit_price_;
}

} // namespace pludux::backtest
