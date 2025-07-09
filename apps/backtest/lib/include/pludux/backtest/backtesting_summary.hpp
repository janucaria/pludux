#ifndef PLUDUX_PLUDUX_BACKTEST_BACKTESTING_SUMMARY_HPP
#define PLUDUX_PLUDUX_BACKTEST_BACKTESTING_SUMMARY_HPP

#include <ctime>
#include <optional>
#include <vector>

#include <pludux/backtest/trade_record.hpp>

namespace pludux::backtest {

class BacktestingSummary {
public:
  auto get_next_summary(TradeRecord trade_record) const noexcept
   -> BacktestingSummary;

  auto trade_record() const noexcept -> const TradeRecord&;

  void trade_record(TradeRecord trade_record) noexcept;

  auto cumulative_investments() const noexcept -> double;

  void cumulative_investments(double investment) noexcept;

  auto cumulative_take_profits() const noexcept -> double;

  void cumulative_take_profits(double take_profit) noexcept;

  auto cumulative_stop_losses() const noexcept -> double;

  void cumulative_stop_losses(double stop_loss) noexcept;

  auto cumulative_exit_wins() const noexcept -> double;

  void cumulative_exit_wins(double exit_win) noexcept;

  auto cumulative_exit_losses() const noexcept -> double;

  void cumulative_exit_losses(double exit_loss) noexcept;

  auto average_investment() const noexcept -> double;

  auto trade_count() const noexcept -> std::size_t;

  auto open_trade_count() const noexcept -> std::size_t;

  auto cumulative_pnls() const noexcept -> double;

  auto cumulative_durations() const noexcept -> std::time_t;

  void cumulative_durations(std::time_t duration) noexcept;

  auto average_duration() const noexcept -> std::time_t;

  auto average_pnl() const noexcept -> double;

  auto take_profit_count() const noexcept -> std::size_t;

  void take_profit_count(std::size_t count) noexcept;

  auto take_profit_rate() const noexcept -> double;

  auto average_take_profit() const noexcept -> double;

  auto take_profit_expected_value() const noexcept -> double;

  auto stop_loss_count() const noexcept -> std::size_t;

  void stop_loss_count(std::size_t count) noexcept;

  auto stop_loss_rate() const noexcept -> double;

  auto average_stop_loss() const noexcept -> double;

  auto stop_loss_expected_value() const noexcept -> double;

  auto exit_win_count() const noexcept -> std::size_t;

  void exit_win_count(std::size_t count) noexcept;

  auto exit_loss_count() const noexcept -> std::size_t;

  void exit_loss_count(std::size_t count) noexcept;

  auto break_even_count() const noexcept -> std::size_t;

  void break_even_count(std::size_t count) noexcept;

  auto exit_signal_count() const noexcept -> std::size_t;

  auto exit_signal_rate() const noexcept -> double;

  auto sum_of_exit_signals() const noexcept -> double;

  auto average_exit_signal() const noexcept -> double;

  auto exit_signal_expected_value() const noexcept -> double;

  auto expected_value() const noexcept -> double;

  auto expected_return() const noexcept -> double;

  auto win_count() const noexcept -> std::size_t;

  auto win_rate() const noexcept -> double;

  auto total_profits() const noexcept -> double;

  auto total_profit_percent() const noexcept -> double;

  auto average_win() const noexcept -> double;

  auto loss_count() const noexcept -> std::size_t;

  auto loss_rate() const noexcept -> double;

  auto total_losses() const noexcept -> double;

  auto total_loss_percent() const noexcept -> double;

  auto average_loss() const noexcept -> double;

  auto break_even_rate() const noexcept -> double;

  auto profit_factor() const noexcept -> double;

  auto unrealized_pnl() const noexcept -> double;

  auto ongoing_trade_duration() const noexcept -> std::time_t;

private:
  TradeRecord trade_record_{};

  std::time_t sum_of_durations_{};
  double cumulative_investments_{};

  std::size_t take_profit_count_{};
  double cumulative_take_profits_{};

  std::size_t stop_loss_count_{};
  double cumulative_stop_losses_{};

  std::size_t exit_win_count_{};
  double cumulative_exit_wins_{};

  std::size_t exit_loss_count_{};
  double cumulative_exit_losses_{};

  std::size_t break_even_count_{};
};

} // namespace pludux::backtest

#endif