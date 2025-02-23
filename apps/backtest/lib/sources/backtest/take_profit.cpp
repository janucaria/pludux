#include <pludux/ta.hpp>

#include <pludux/backtest/take_profit.hpp>

namespace pludux::backtest {

TakeProfit::TakeProfit(screener::ScreenerMethod reward_method,
                       bool is_disabled,
                       screener::ScreenerMethod trading_price_method)
: reward_method_{std::move(reward_method)}
, is_disabled_{is_disabled}
, trading_price_method_{std::move(trading_price_method)}
{
}

auto TakeProfit::operator()(const RunningState& running_state) const
 -> TradingTakeProfit
{
  const auto asset = running_state.asset_snapshot();
  const auto entry_price = running_state.price();
  const auto take_profit = reward_method_(asset)[0];
  const auto take_profit_price = entry_price + take_profit;

  return TradingTakeProfit{
   is_disabled_, take_profit_price, trading_price_method_};
}

} // namespace pludux::backtest
