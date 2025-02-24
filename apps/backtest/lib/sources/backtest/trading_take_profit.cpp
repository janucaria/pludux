#include <pludux/backtest/trading_take_profit.hpp>

namespace pludux::backtest {

TradingTakeProfit::TradingTakeProfit(
 bool is_disabled,
 double profit_price,
 screener::ScreenerMethod signal_price_method)
: is_disabled_{is_disabled}
, profit_price_{profit_price}
, signal_price_method_{std::move(signal_price_method)}
{
}

auto TradingTakeProfit::exit_price() const noexcept -> double
{
  return !is_disabled_ ? profit_price_
                       : std::numeric_limits<double>::quiet_NaN();
}

auto TradingTakeProfit::operator()(const AssetSnapshot& asset) -> bool
{
  const auto signal_price = signal_price_method_(asset)[0];
  const auto reference_price = exit_price();
  return signal_price >= reference_price;
}

} // namespace pludux::backtest
