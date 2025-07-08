#ifndef PLUDUX_PLUDUX_BACKTEST_STOP_LOSS_HPP
#define PLUDUX_PLUDUX_BACKTEST_STOP_LOSS_HPP

import pludux.asset_history;
import pludux.asset_snapshot;

#include <pludux/screener.hpp>

#include <pludux/backtest/trading_stop_loss.hpp>

namespace pludux::backtest {

class StopLoss {
public:
  StopLoss(screener::ScreenerMethod risk_method,
           bool is_disabled,
           bool is_trailing = false);

  auto operator()(AssetSnapshot asset_snapshot) const -> TradingStopLoss;

  auto is_disabled() const noexcept -> bool;

  auto is_trailing() const noexcept -> bool;

  auto risk_method() const noexcept -> const screener::ScreenerMethod&;

  auto get_risk_size(AssetSnapshot asset_snapshot) const -> double;

private:
  bool is_disabled_;
  bool is_trailing_;
  screener::ScreenerMethod risk_method_;
};

} // namespace pludux::backtest

#endif