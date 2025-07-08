#ifndef PLUDUX_PLUDUX_BACKTEST_TAKE_PROFIT_HPP
#define PLUDUX_PLUDUX_BACKTEST_TAKE_PROFIT_HPP

import pludux.asset_snapshot;

#include <pludux/screener.hpp>
#include <pludux/backtest/trading_take_profit.hpp>

namespace pludux::backtest {

class TakeProfit {
public:
  TakeProfit(screener::ScreenerMethod reward_method,
             bool is_disabled,
             screener::ScreenerMethod trading_price_method);

  auto operator()(AssetSnapshot asset_snapshot) const -> TradingTakeProfit;

  auto is_disabled() const noexcept -> bool;

  auto reward_method() const noexcept -> const screener::ScreenerMethod&;

  auto trading_price_method() const noexcept -> const screener::ScreenerMethod&;

private:
  bool is_disabled_;
  screener::ScreenerMethod reward_method_;
  screener::ScreenerMethod trading_price_method_;
};

} // namespace pludux::backtest

#endif