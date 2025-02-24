#ifndef PLUDUX_PLUDUX_BACKTEST_TRADING_STOP_LOSS_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADING_STOP_LOSS_HPP

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener.hpp>

#include <pludux/backtest/running_state.hpp>

namespace pludux::backtest {

class TradingStopLoss {
public:
  TradingStopLoss(bool is_disabled,
                  bool is_trailing,
                  double risk,
                  double stop_price);

  auto exit_price() const noexcept -> double;

  auto operator()(const RunningState& running_state) -> bool;

private:
  bool is_disabled_;
  bool is_trailing_;
  double risk_;
  double stop_price_;
};

} // namespace pludux::backtest

#endif