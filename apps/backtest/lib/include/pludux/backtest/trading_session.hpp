#ifndef PLUDUX_PLUDUX_BACKTEST_TRADING_SESSION_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADING_SESSION_HPP

#include <ctime>
#include <optional>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener.hpp>

#include <pludux/backtest/trade_record.hpp>
#include <pludux/backtest/trading_stop_loss.hpp>
#include <pludux/backtest/trading_take_profit.hpp>

namespace pludux::backtest {

class TradingSession {
public:
  TradingSession(double order_size,
                 std::size_t entry_index,
                 double entry_price,
                 std::time_t entry_timestamp,
                 screener::ScreenerMethod timestamp_method,
                 screener::ScreenerMethod price_method,
                 screener::ScreenerFilter exit_filter,
                 TradingTakeProfit trading_take_profit,
                 TradingStopLoss trading_stop_loss);

  auto done_trade(const AssetSnapshot& asset) -> std::optional<TradeRecord>;

  auto ongoing_trade(const AssetSnapshot& asset) const -> TradeRecord;

private:
  double order_size_;
  double entry_price_;
  std::size_t entry_index_;
  std::time_t entry_timestamp_;
  screener::ScreenerMethod timestamp_method_;
  screener::ScreenerMethod price_method_;
  screener::ScreenerFilter exit_filter_;
  TradingTakeProfit trading_take_profit_;
  TradingStopLoss trading_stop_loss_;
};

} // namespace pludux::backtest

#endif