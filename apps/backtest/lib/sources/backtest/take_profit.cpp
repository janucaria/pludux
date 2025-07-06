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

auto TakeProfit::operator()(AssetSnapshot asset_snapshot) const
 -> TradingTakeProfit
{
  const auto entry_price = asset_snapshot.get_close();
  const auto take_profit = reward_method_(asset_snapshot)[0];
  const auto take_profit_price = entry_price + take_profit;

  return TradingTakeProfit{
   is_disabled_, take_profit_price, trading_price_method_};
}

auto TakeProfit::is_disabled() const noexcept -> bool
{
  return is_disabled_;
}

auto TakeProfit::reward_method() const noexcept
 -> const screener::ScreenerMethod&
{
  return reward_method_;
}

auto TakeProfit::trading_price_method() const noexcept
 -> const screener::ScreenerMethod&
{
  return trading_price_method_;
}

} // namespace pludux::backtest
