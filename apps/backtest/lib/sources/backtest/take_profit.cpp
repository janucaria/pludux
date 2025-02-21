#include <pludux/ta.hpp>

#include <pludux/backtest/take_profit.hpp>

namespace pludux::backtest {

TakeProfit::TakeProfit()
: TakeProfit{
   screener::ValueMethod{0}, screener::ValueMethod{0}, screener::ValueMethod{0}}
{
}

TakeProfit::TakeProfit(screener::ScreenerMethod entry_price_method,
                       screener::ScreenerMethod reward_method,
                       screener::ScreenerMethod trading_price_method)
: entry_price_method_{entry_price_method}
, reward_method_{std::move(reward_method)}
, trading_price_method_{std::move(trading_price_method)}
{
}

auto TakeProfit::operator()(const AssetSnapshot& asset) const
 -> TradingTakeProfit
{
  const auto entry_price = entry_price_method_(asset)[0];
  const auto take_profit = reward_method_(asset)[0];
  const auto take_profit_price = entry_price + take_profit;

  return TradingTakeProfit{take_profit_price, trading_price_method_};
}

} // namespace pludux::backtest
