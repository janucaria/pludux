#ifndef PLUDUX_PLUDUX_BACKTEST_BACKTESTING_SUMMARY_HPP
#define PLUDUX_PLUDUX_BACKTEST_BACKTESTING_SUMMARY_HPP

#include <ctime>
#include <optional>
#include <vector>

#include <pludux/backtest/trade_record.hpp>

namespace pludux::backtest {

class BacktestingSummary {
public:
  BacktestingSummary();

  auto open_trade() const -> const std::optional<TradeRecord>&;

  auto closed_trades() const -> const std::vector<TradeRecord>&;

  void add_trade(TradeRecord trade);

  auto total_profit() const -> double;

  auto total_duration() const -> std::time_t;

  auto total_trades() const -> std::size_t;

  auto average_duration() const -> std::time_t;

  auto unrealized_profit() const -> double;

  auto ongoing_trade_duration() const -> std::time_t;

  auto take_profit_rate() const -> double;

  auto average_take_profit() const -> double;

  auto take_profit_expected_value() const -> double;

  auto stop_loss_rate() const -> double;

  auto average_stop_loss() const -> double;

  auto stop_loss_expected_value() const -> double;

  auto exit_signal_rate() const -> double;

  auto average_exit_signal() const -> double;

  auto exit_signal_expected_value() const -> double;

  auto expected_value() const -> double;

  auto win_rate() const -> double;

  auto loss_rate() const -> double;

  auto break_even_rate() const -> double;

  auto average_win() const -> double;

  auto average_loss() const -> double;

private:
  std::optional<TradeRecord> open_trade_;
  std::vector<TradeRecord> closed_trades_;
};

} // namespace pludux::backtest

#endif