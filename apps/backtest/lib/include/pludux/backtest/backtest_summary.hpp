#ifndef PLUDUX_PLUDUX_BACKTEST_BACKTEST_SUMMARY_HPP
#define PLUDUX_PLUDUX_BACKTEST_BACKTEST_SUMMARY_HPP

#include <ctime>
#include <optional>
#include <vector>

#include <pludux/backtest/trade_session.hpp>

namespace pludux::backtest {

class BacktestSummary {
public:
  BacktestSummary();

  explicit BacktestSummary(double initial_capital,
                           TradeSession trade_session = TradeSession{});

  void update_to_next_summary(TradeSession next_trade_session) noexcept;

  auto trade_session() const noexcept -> const TradeSession&;

  void trade_session(TradeSession trade_session) noexcept;

  auto capital() const noexcept -> double;

  void capital(double capital) noexcept;

  auto peak_equity() const noexcept -> double;

  void peak_equity(double peak_equity) noexcept;

  auto max_drawdown() const noexcept -> double;

  void max_drawdown(double max_drawdown) noexcept;

  auto cumulative_durations() const noexcept -> std::time_t;

  void cumulative_durations(std::time_t duration) noexcept;

  auto cumulative_investments() const noexcept -> double;

  void cumulative_investments(double investment) noexcept;

  auto cumulative_profits() const noexcept -> double;

  void cumulative_profits(double profits) noexcept;

  auto cumulative_losses() const noexcept -> double;

  void cumulative_losses(double losses) noexcept;

  auto profit_count() const noexcept -> std::size_t;

  void profit_count(std::size_t count) noexcept;

  auto loss_count() const noexcept -> std::size_t;

  void loss_count(std::size_t count) noexcept;

  auto break_even_count() const noexcept -> std::size_t;

  void break_even_count(std::size_t count) noexcept;

  auto average_investment() const noexcept -> double;

  auto trade_count() const noexcept -> std::size_t;

  auto open_trade_count() const noexcept -> std::size_t;

  auto cumulative_pnls() const noexcept -> double;

  auto average_duration() const noexcept -> std::time_t;

  auto average_pnl() const noexcept -> double;

  auto expected_value() const noexcept -> double;

  auto expected_return() const noexcept -> double;

  auto profit_rate() const noexcept -> double;

  auto cumulative_profit_percent() const noexcept -> double;

  auto average_profit() const noexcept -> double;

  auto loss_rate() const noexcept -> double;

  auto cumulative_loss_percent() const noexcept -> double;

  auto average_loss() const noexcept -> double;

  auto break_even_rate() const noexcept -> double;

  auto profit_factor() const noexcept -> double;

  auto unrealized_pnl() const noexcept -> double;

  auto unrealized_investment() const noexcept -> double;

  auto unrealized_duration() const noexcept -> std::time_t;

  auto equity() const noexcept -> double;

  auto initial_capital() const noexcept -> double;

  auto drawdown() const noexcept -> double;

private:
  TradeSession trade_session_;

  double capital_;
  double peak_equity_;
  double max_drawdown_;

  std::time_t sum_of_durations_;
  double cumulative_investments_;

  std::size_t profit_count_;
  double cumulative_profits_;

  std::size_t loss_count_;
  double cumulative_losses_;

  std::size_t break_even_count_;
};

} // namespace pludux::backtest

#endif