#ifndef PLUDUX_PLUDUX_BACKTEST_TRADING_STOP_LOSS_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADING_STOP_LOSS_HPP

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener.hpp>

namespace pludux::backtest {

class TradingStopLoss {
public:
  TradingStopLoss(double stop_price,
                  screener::ScreenerMethod asset_price_method);

  auto exit_price() const noexcept -> double;

  auto operator()(const AssetSnapshot& asset) -> bool;

private:
  double stop_price_;
  screener::ScreenerMethod asset_price_method_;
};

} // namespace pludux::backtest

#endif