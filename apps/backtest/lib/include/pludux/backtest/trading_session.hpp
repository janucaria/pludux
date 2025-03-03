#ifndef PLUDUX_PLUDUX_BACKTEST_TRADING_SESSION_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADING_SESSION_HPP

#include <ctime>
#include <optional>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener.hpp>

#include <pludux/backtest/trade_record.hpp>
#include <pludux/backtest/trading_stop_loss.hpp>
#include <pludux/backtest/trading_take_profit.hpp>

#include "./running_state.hpp"

namespace pludux::backtest {

class TradingSession {
public:
  TradingSession(double order_size,
                 std::size_t entry_index,
                 double entry_price,
                 std::time_t entry_timestamp,
                 screener::ScreenerFilter exit_filter,
                 TradingTakeProfit trading_take_profit,
                 TradingStopLoss trading_stop_loss);

  auto get_trading_state(const RunningState& running_state) -> TradeRecord;

private:
  double order_size_;
  double entry_price_;
  std::size_t entry_index_;
  std::time_t entry_timestamp_;
  screener::ScreenerFilter exit_filter_;
  TradingTakeProfit trading_take_profit_;
  TradingStopLoss trading_stop_loss_;
};

} // namespace pludux::backtest

#endif