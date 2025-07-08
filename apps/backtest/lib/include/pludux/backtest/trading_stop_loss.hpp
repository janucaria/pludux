#ifndef PLUDUX_PLUDUX_BACKTEST_TRADING_STOP_LOSS_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADING_STOP_LOSS_HPP

import pludux.asset_snapshot;

#include <pludux/screener.hpp>

namespace pludux::backtest {

class TradingStopLoss {
public:
  TradingStopLoss(bool is_disabled,
                  bool is_trailing,
                  double risk,
                  double stop_price);

  auto is_trailing() const noexcept -> bool;

  auto initial_exit_price() const noexcept -> double;

  auto exit_price() const noexcept -> double;

  auto operator()(const AssetSnapshot& asset_snapshot) -> bool;

  auto stop_price() const noexcept -> double;

  void stop_price(double new_stop_price) noexcept;

private:
  bool is_disabled_;
  bool is_trailing_;
  double risk_;
  double stop_price_;
  double initial_stop_price_;
};

} // namespace pludux::backtest

#endif