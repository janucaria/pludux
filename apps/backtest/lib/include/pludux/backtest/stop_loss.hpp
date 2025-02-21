#ifndef PLUDUX_PLUDUX_BACKTEST_STOP_LOSS_HPP
#define PLUDUX_PLUDUX_BACKTEST_STOP_LOSS_HPP

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener.hpp>

#include <pludux/backtest/trading_stop_loss.hpp>


#include "./running_state.hpp"

namespace pludux::backtest {

class StopLoss {
public:
  StopLoss();

  StopLoss(screener::ScreenerMethod risk_method,
           screener::ScreenerMethod trading_price_method);

  auto operator()(const RunningState& running_state) const -> TradingStopLoss;

  auto get_order_size(const RunningState& running_state) const
   -> double;

private:
  screener::ScreenerMethod risk_method_;
  screener::ScreenerMethod trading_price_method_;
};

} // namespace pludux::backtest

#endif