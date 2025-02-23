#ifndef PLUDUX_PLUDUX_BACKTEST_TAKE_PROFIT_HPP
#define PLUDUX_PLUDUX_BACKTEST_TAKE_PROFIT_HPP

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener.hpp>

#include <pludux/backtest/trading_take_profit.hpp>

#include "./running_state.hpp"

namespace pludux::backtest {

class TakeProfit {
public:
  TakeProfit(screener::ScreenerMethod reward_method,
             bool is_disabled,
             screener::ScreenerMethod trading_price_method);

  auto operator()(const RunningState& running_state) const -> TradingTakeProfit;

private:
  bool is_disabled_;
  screener::ScreenerMethod reward_method_;
  screener::ScreenerMethod trading_price_method_;
};

} // namespace pludux::backtest

#endif