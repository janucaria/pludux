#include <pludux/backtest/trading_take_profit.hpp>

namespace pludux::backtest {

TradingTakeProfit::TradingTakeProfit(
 double profit_price, screener::ScreenerMethod asset_price_method)
: profit_price_{profit_price}
, asset_price_method_{std::move(asset_price_method)}
{
}

auto TradingTakeProfit::exit_price() const noexcept -> double
{
  return profit_price_;
}

auto TradingTakeProfit::operator()(const AssetSnapshot& asset) -> bool
{
  const auto price = asset_price_method_(asset)[0];
  return price >= profit_price_;
}

} // namespace pludux::backtest
