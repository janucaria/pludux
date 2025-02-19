#ifndef PLUDUX_PLUDUX_BACKTEST_TAKE_PROFIT_HPP
#define PLUDUX_PLUDUX_BACKTEST_TAKE_PROFIT_HPP

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener.hpp>

#include <pludux/backtest/trading_take_profit.hpp>

namespace pludux::backtest {

class TakeProfit {
public:
  TakeProfit();

  TakeProfit(screener::ScreenerMethod entry_price_method,
             screener::ScreenerMethod reward_method,
             screener::ScreenerMethod trading_price_method);

  auto operator()(const AssetSnapshot& asset) const -> TradingTakeProfit;

private:
  screener::ScreenerMethod entry_price_method_;
  screener::ScreenerMethod reward_method_;
  screener::ScreenerMethod trading_price_method_;
};

} // namespace pludux::backtest

#endif