#ifndef PLUDUX_PLUDUX_BACKTEST_TRADING_TAKE_PROFIT_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADING_TAKE_PROFIT_HPP

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener.hpp>

namespace pludux::backtest {

class TradingTakeProfit {
public:
  TradingTakeProfit(bool is_disabled,
                    bool is_short_position,
                    double profit_price,
                    screener::ScreenerMethod signal_price_method);

  auto exit_price() const noexcept -> double;

  auto operator()(const AssetSnapshot& asset) -> bool;

private:
  bool is_disabled_;
  bool is_short_position_;
  double profit_price_;
  screener::ScreenerMethod signal_price_method_;
};

} // namespace pludux::backtest

#endif