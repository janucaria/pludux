module;

#include <ctime>
#include <numeric>
#include <optional>
#include <utility>
#include <vector>

export module pludux.backtest:backtest_summary;

import :trade_session;

export namespace pludux::backtest {

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

// ------------------------------------------------------------------------

BacktestSummary::BacktestSummary()
: BacktestSummary{0.0, TradeSession{}}
{
}

BacktestSummary::BacktestSummary(double initial_capital,
                                 TradeSession trade_session)
: trade_session_{std::move(trade_session)}
, capital_{initial_capital}
, peak_equity_{initial_capital}
, max_drawdown_{0.0}
, sum_of_durations_{}
, cumulative_investments_{0.0}
, profit_count_{0}
, cumulative_profits_{0.0}
, loss_count_{0}
, cumulative_losses_{0.0}
, break_even_count_{0}
{
}

void BacktestSummary::update_to_next_summary(
 TradeSession next_trade_session) noexcept
{
  if(next_trade_session.closed_position()) {
    const auto pnl = next_trade_session.realized_pnl();

    sum_of_durations_ += next_trade_session.realized_duration();
    cumulative_investments_ += next_trade_session.realized_investment();

    if(pnl > 0) {
      profit_count_++;
      cumulative_profits_ += pnl;
    } else if(pnl < 0) {
      loss_count_++;
      cumulative_losses_ += pnl;
    } else {
      break_even_count_++;
    }

    capital_ += pnl;
  }

  trade_session_ = std::move(next_trade_session);
  peak_equity_ = std::max(peak_equity_, equity());
  max_drawdown_ = std::max(max_drawdown_, drawdown());
}

auto BacktestSummary::trade_session() const noexcept -> const TradeSession&
{
  return trade_session_;
}

void BacktestSummary::trade_session(TradeSession trade_session) noexcept
{
  trade_session_ = std::move(trade_session);
}

auto BacktestSummary::capital() const noexcept -> double
{
  return capital_;
}

void BacktestSummary::capital(double capital) noexcept
{
  capital_ = capital;
}

auto BacktestSummary::peak_equity() const noexcept -> double
{
  return peak_equity_;
}

void BacktestSummary::peak_equity(double peak_equity) noexcept
{
  peak_equity_ = peak_equity;
}

auto BacktestSummary::max_drawdown() const noexcept -> double
{
  return max_drawdown_;
}

void BacktestSummary::max_drawdown(double max_drawdown) noexcept
{
  max_drawdown_ = max_drawdown;
}

auto BacktestSummary::cumulative_investments() const noexcept -> double
{
  return cumulative_investments_;
}

void BacktestSummary::cumulative_investments(double investment) noexcept
{
  cumulative_investments_ = investment;
}

auto BacktestSummary::average_investment() const noexcept -> double
{
  return trade_count() ? cumulative_investments() / trade_count() : 0.0;
}

auto BacktestSummary::trade_count() const noexcept -> std::size_t
{
  return profit_count() + loss_count() + break_even_count_;
}

auto BacktestSummary::open_trade_count() const noexcept -> std::size_t
{
  return trade_session_.is_open() ? 1 : 0;
}

auto BacktestSummary::cumulative_pnls() const noexcept -> double
{
  return cumulative_profits() + cumulative_losses();
}

auto BacktestSummary::cumulative_durations() const noexcept -> std::time_t
{
  return sum_of_durations_;
}

void BacktestSummary::cumulative_durations(std::time_t duration) noexcept
{
  sum_of_durations_ = duration;
}

auto BacktestSummary::average_duration() const noexcept -> std::time_t
{
  return trade_count() ? cumulative_durations() / trade_count() : 0;
}

auto BacktestSummary::average_pnl() const noexcept -> double
{
  return trade_count() ? cumulative_pnls() / trade_count() : 0.0;
}

auto BacktestSummary::expected_value() const noexcept -> double
{
  return profit_rate() * average_profit() + loss_rate() * average_loss();
}

auto BacktestSummary::expected_return() const noexcept -> double
{
  return average_investment() ? expected_value() / average_investment() * 100
                              : 0.0;
}

auto BacktestSummary::profit_count() const noexcept -> std::size_t
{
  return profit_count_;
}

void BacktestSummary::profit_count(std::size_t count) noexcept
{
  profit_count_ = count;
}

auto BacktestSummary::profit_rate() const noexcept -> double
{
  return trade_count() ? static_cast<double>(profit_count()) / trade_count()
                       : 0.0;
}

auto BacktestSummary::cumulative_profits() const noexcept -> double
{
  return cumulative_profits_;
}

void BacktestSummary::cumulative_profits(double profits) noexcept
{
  cumulative_profits_ = profits;
}

auto BacktestSummary::cumulative_profit_percent() const noexcept -> double
{
  return cumulative_investments()
          ? cumulative_profits() / cumulative_investments() * 100.0
          : 0.0;
}

auto BacktestSummary::average_profit() const noexcept -> double
{
  return profit_count() ? cumulative_profits() / profit_count() : 0.0;
}

auto BacktestSummary::loss_count() const noexcept -> std::size_t
{
  return loss_count_;
}

void BacktestSummary::loss_count(std::size_t count) noexcept
{
  loss_count_ = count;
}

auto BacktestSummary::loss_rate() const noexcept -> double
{
  return trade_count() ? static_cast<double>(loss_count()) / trade_count()
                       : 0.0;
}

auto BacktestSummary::cumulative_losses() const noexcept -> double
{
  return cumulative_losses_;
}

void BacktestSummary::cumulative_losses(double losses) noexcept
{
  cumulative_losses_ = losses;
}

auto BacktestSummary::cumulative_loss_percent() const noexcept -> double
{
  return cumulative_investments()
          ? cumulative_losses() / cumulative_investments() * 100.0
          : 0.0;
}

auto BacktestSummary::average_loss() const noexcept -> double
{
  return loss_count() ? cumulative_losses() / loss_count() : 0.0;
}

auto BacktestSummary::break_even_count() const noexcept -> std::size_t
{
  return break_even_count_;
}

void BacktestSummary::break_even_count(std::size_t count) noexcept
{
  break_even_count_ = count;
}

auto BacktestSummary::break_even_rate() const noexcept -> double
{
  return trade_count() ? static_cast<double>(break_even_count()) / trade_count()
                       : 0.0;
}

auto BacktestSummary::profit_factor() const noexcept -> double
{
  if(cumulative_losses() == 0.0) {
    return std::numeric_limits<double>::infinity();
  }
  return cumulative_profits() / -cumulative_losses();
}

auto BacktestSummary::unrealized_pnl() const noexcept -> double
{
  return trade_session_.unrealized_pnl();
}

auto BacktestSummary::unrealized_investment() const noexcept -> double
{
  return trade_session_.unrealized_investment();
}

auto BacktestSummary::unrealized_duration() const noexcept -> std::time_t
{
  return trade_session_.unrealized_duration();
}

auto BacktestSummary::equity() const noexcept -> double
{
  return capital() + trade_session_.partial_realized_pnl() +
         trade_session_.unrealized_pnl();
}

auto BacktestSummary::initial_capital() const noexcept -> double
{
  return capital_ - cumulative_pnls();
}

auto BacktestSummary::drawdown() const noexcept -> double
{
  return peak_equity_ ? (peak_equity_ - equity()) / peak_equity_ * 100.0 : 0.0;
}

} // namespace pludux::backtest
