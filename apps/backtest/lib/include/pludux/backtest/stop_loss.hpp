#ifndef PLUDUX_PLUDUX_BACKTEST_STOP_LOSS_HPP
#define PLUDUX_PLUDUX_BACKTEST_STOP_LOSS_HPP

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener.hpp>

#include <pludux/backtest/trading_stop_loss.hpp>

#include "./running_state.hpp"

namespace pludux::backtest {

class StopLoss {
public:
  StopLoss(screener::ScreenerMethod risk_method,
           bool is_disabled,
           bool is_trailing = false);

  auto operator()(const RunningState& running_state) const -> TradingStopLoss;

  auto is_disabled() const noexcept -> bool;

  auto is_trailing() const noexcept -> bool;

  auto risk_method() const noexcept -> const screener::ScreenerMethod&;

  auto get_order_size(const RunningState& running_state) const -> double;

private:
  bool is_disabled_;
  bool is_trailing_;
  screener::ScreenerMethod risk_method_;
};

} // namespace pludux::backtest

#endif