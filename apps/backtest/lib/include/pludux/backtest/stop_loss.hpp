#ifndef PLUDUX_PLUDUX_BACKTEST_STOP_LOSS_HPP
#define PLUDUX_PLUDUX_BACKTEST_STOP_LOSS_HPP

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener.hpp>

#include <pludux/backtest/trading_stop_loss.hpp>

namespace pludux::backtest {

class StopLoss {
public:
  StopLoss();

  StopLoss(screener::ScreenerMethod entry_price_method,
           screener::ScreenerMethod risk_method,
           screener::ScreenerMethod trading_price_method);

  auto operator()(const AssetSnapshot& asset) const -> TradingStopLoss;

  auto get_order_size(double capital_risk, const AssetSnapshot& asset) const
   -> double;

private:
  screener::ScreenerMethod entry_price_method_;
  screener::ScreenerMethod risk_method_;
  screener::ScreenerMethod trading_price_method_;
};

} // namespace pludux::backtest

#endif