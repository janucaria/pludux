#include <numeric>
#include <utility>

#include <pludux/backtest/backtesting_summary.hpp>

namespace pludux::backtest {

BacktestingSummary::BacktestingSummary()
: BacktestingSummary{0.0, TradeSession{}}
{
}

BacktestingSummary::BacktestingSummary(double initial_capital,
                                       TradeSession trade_session)
: trade_session_{std::move(trade_session)}
, capital_{initial_capital}
, peak_equity_{initial_capital}
, max_drawdown_{0.0}
, sum_of_durations_{}
, cumulative_investments_{0.0}
, take_profit_count_{0}
, cumulative_take_profits_{0.0}
, stop_loss_profit_count_{0}
, cumulative_stop_loss_profits_{0.0}
, stop_loss_loss_count_{0}
, cumulative_stop_loss_losses_{0.0}
, exit_signal_profit_count_{0}
, cumulative_exit_signal_profits_{0.0}
, exit_signal_loss_count_{0}
, cumulative_exit_signal_losses_{0.0}
, break_even_count_{0}
{
}

void BacktestingSummary::update_to_next_summary(
 TradeSession next_trade_session) noexcept
{
  if(next_trade_session.is_closed()) {
    const auto pnl = next_trade_session.pnl();

    sum_of_durations_ += next_trade_session.duration();
    cumulative_investments_ += next_trade_session.investment();

    if(next_trade_session.is_closed_take_profit()) {
      take_profit_count_++;
      cumulative_take_profits_ += pnl;
    } else if(next_trade_session.is_closed_stop_loss()) {
      if(pnl > 0) {
        stop_loss_profit_count_++;
        cumulative_stop_loss_profits_ += pnl;
      } else if(pnl < 0) {
        stop_loss_loss_count_++;
        cumulative_stop_loss_losses_ += pnl;
      } else {
        break_even_count_++;
      }
    } else if(next_trade_session.is_closed_exit_signal()) {
      if(pnl > 0) {
        exit_signal_profit_count_++;
        cumulative_exit_signal_profits_ += pnl;
      } else if(pnl < 0) {
        exit_signal_loss_count_++;
        cumulative_exit_signal_losses_ += pnl;
      } else {
        break_even_count_++;
      }
    }

    capital_ += pnl;
  }

  trade_session_ = std::move(next_trade_session);
  peak_equity_ = std::max(peak_equity_, equity());
  max_drawdown_ = std::max(max_drawdown_, drawdown());
}

auto BacktestingSummary::trade_session() const noexcept -> const TradeSession&
{
  return trade_session_;
}

void BacktestingSummary::trade_session(TradeSession trade_session) noexcept
{
  trade_session_ = std::move(trade_session);
}

auto BacktestingSummary::capital() const noexcept -> double
{
  return capital_;
}

void BacktestingSummary::capital(double capital) noexcept
{
  capital_ = capital;
}

auto BacktestingSummary::peak_equity() const noexcept -> double
{
  return peak_equity_;
}

void BacktestingSummary::peak_equity(double peak_equity) noexcept
{
  peak_equity_ = peak_equity;
}

auto BacktestingSummary::max_drawdown() const noexcept -> double
{
  return max_drawdown_;
}

void BacktestingSummary::max_drawdown(double max_drawdown) noexcept
{
  max_drawdown_ = max_drawdown;
}

auto BacktestingSummary::cumulative_investments() const noexcept -> double
{
  return cumulative_investments_;
}

void BacktestingSummary::cumulative_investments(double investment) noexcept
{
  cumulative_investments_ = investment;
}

auto BacktestingSummary::cumulative_take_profits() const noexcept -> double
{
  return cumulative_take_profits_;
}

void BacktestingSummary::cumulative_take_profits(double take_profit) noexcept
{
  cumulative_take_profits_ = take_profit;
}

auto BacktestingSummary::cumulative_stop_loss_profits() const noexcept -> double
{
  return cumulative_stop_loss_profits_;
}

void BacktestingSummary::cumulative_stop_loss_profits(
 double stop_loss_profit) noexcept
{
  cumulative_stop_loss_profits_ = stop_loss_profit;
}

auto BacktestingSummary::cumulative_stop_loss_losses() const noexcept -> double
{
  return cumulative_stop_loss_losses_;
}

void BacktestingSummary::cumulative_stop_loss_losses(
 double stop_loss_loss) noexcept
{
  cumulative_stop_loss_losses_ = stop_loss_loss;
}

auto BacktestingSummary::cumulative_stop_losses() const noexcept -> double
{
  return cumulative_stop_loss_profits() + cumulative_stop_loss_losses();
}

auto BacktestingSummary::cumulative_exit_signal_profits() const noexcept
 -> double
{
  return cumulative_exit_signal_profits_;
}

void BacktestingSummary::cumulative_exit_signal_profits(
 double exit_profit) noexcept
{
  cumulative_exit_signal_profits_ = exit_profit;
}

auto BacktestingSummary::cumulative_exit_signal_losses() const noexcept
 -> double
{
  return cumulative_exit_signal_losses_;
}

void BacktestingSummary::cumulative_exit_signal_losses(
 double exit_loss) noexcept
{
  cumulative_exit_signal_losses_ = exit_loss;
}

auto BacktestingSummary::average_investment() const noexcept -> double
{
  return trade_count() ? cumulative_investments() / trade_count() : 0.0;
}

auto BacktestingSummary::trade_count() const noexcept -> std::size_t
{
  return profit_count() + loss_count() + break_even_count_;
}

auto BacktestingSummary::open_trade_count() const noexcept -> std::size_t
{
  return trade_session_.is_open() ? 1 : 0;
}

auto BacktestingSummary::cumulative_pnls() const noexcept -> double
{
  return cumulative_take_profits() + cumulative_stop_losses() +
         sum_of_exit_signals();
}

auto BacktestingSummary::cumulative_durations() const noexcept -> std::time_t
{
  return sum_of_durations_;
}

void BacktestingSummary::cumulative_durations(std::time_t duration) noexcept
{
  sum_of_durations_ = duration;
}

auto BacktestingSummary::average_duration() const noexcept -> std::time_t
{
  return trade_count() ? cumulative_durations() / trade_count() : 0;
}

auto BacktestingSummary::average_pnl() const noexcept -> double
{
  return trade_count() ? cumulative_pnls() / trade_count() : 0.0;
}

auto BacktestingSummary::take_profit_count() const noexcept -> std::size_t
{
  return take_profit_count_;
}

void BacktestingSummary::take_profit_count(std::size_t count) noexcept
{
  take_profit_count_ = count;
}

auto BacktestingSummary::take_profit_rate() const noexcept -> double
{
  return trade_count()
          ? static_cast<double>(take_profit_count()) / trade_count()
          : 0.0;
}

auto BacktestingSummary::average_take_profit() const noexcept -> double
{
  return take_profit_count() ? cumulative_take_profits() / take_profit_count()
                             : 0.0;
}

auto BacktestingSummary::take_profit_expected_value() const noexcept -> double
{
  return take_profit_rate() * average_take_profit();
}

auto BacktestingSummary::stop_loss_profit_count() const noexcept -> std::size_t
{
  return stop_loss_profit_count_;
}

void BacktestingSummary::stop_loss_profit_count(std::size_t count) noexcept
{
  stop_loss_profit_count_ = count;
}

auto BacktestingSummary::stop_loss_loss_count() const noexcept -> std::size_t
{
  return stop_loss_loss_count_;
}

void BacktestingSummary::stop_loss_loss_count(std::size_t count) noexcept
{
  stop_loss_loss_count_ = count;
}

auto BacktestingSummary::stop_loss_count() const noexcept -> std::size_t
{
  return stop_loss_profit_count() + stop_loss_loss_count();
}

auto BacktestingSummary::stop_loss_rate() const noexcept -> double
{
  return trade_count() ? static_cast<double>(stop_loss_count()) / trade_count()
                       : 0.0;
}

auto BacktestingSummary::average_stop_loss() const noexcept -> double
{
  return stop_loss_count() ? cumulative_stop_losses() / stop_loss_count() : 0.0;
}

auto BacktestingSummary::stop_loss_expected_value() const noexcept -> double
{
  return stop_loss_rate() * average_stop_loss();
}

auto BacktestingSummary::exit_signal_profit_count() const noexcept
 -> std::size_t
{
  return exit_signal_profit_count_;
}

void BacktestingSummary::exit_signal_profit_count(std::size_t count) noexcept
{
  exit_signal_profit_count_ = count;
}

auto BacktestingSummary::exit_signal_loss_count() const noexcept -> std::size_t
{
  return exit_signal_loss_count_;
}

void BacktestingSummary::exit_signal_loss_count(std::size_t count) noexcept
{
  exit_signal_loss_count_ = count;
}

auto BacktestingSummary::exit_signal_count() const noexcept -> std::size_t
{
  return exit_signal_profit_count() + exit_signal_loss_count() +
         break_even_count();
}

auto BacktestingSummary::exit_signal_rate() const noexcept -> double
{
  return trade_count()
          ? static_cast<double>(exit_signal_count()) / trade_count()
          : 0.0;
}

auto BacktestingSummary::sum_of_exit_signals() const noexcept -> double
{
  return cumulative_exit_signal_profits() + cumulative_exit_signal_losses();
}

auto BacktestingSummary::average_exit_signal() const noexcept -> double
{
  return exit_signal_count() ? sum_of_exit_signals() / exit_signal_count()
                             : 0.0;
}

auto BacktestingSummary::exit_signal_expected_value() const noexcept -> double
{
  return exit_signal_rate() * average_exit_signal();
}

auto BacktestingSummary::expected_value() const noexcept -> double
{
  return take_profit_expected_value() + stop_loss_expected_value() +
         exit_signal_expected_value();
}

auto BacktestingSummary::expected_return() const noexcept -> double
{
  return average_investment() ? expected_value() / average_investment() * 100
                              : 0.0;
}

auto BacktestingSummary::profit_count() const noexcept -> std::size_t
{
  return take_profit_count() + stop_loss_profit_count() +
         exit_signal_profit_count();
}

auto BacktestingSummary::profit_rate() const noexcept -> double
{
  return trade_count() ? static_cast<double>(profit_count()) / trade_count()
                       : 0.0;
}

auto BacktestingSummary::cumulative_profits() const noexcept -> double
{
  return cumulative_take_profits() + cumulative_stop_loss_profits() +
         cumulative_exit_signal_profits();
}

auto BacktestingSummary::cumulative_profit_percent() const noexcept -> double
{
  return cumulative_investments()
          ? cumulative_profits() / cumulative_investments() * 100.0
          : 0.0;
}

auto BacktestingSummary::average_profit() const noexcept -> double
{
  return profit_count() ? cumulative_profits() / profit_count() : 0.0;
}

auto BacktestingSummary::loss_count() const noexcept -> std::size_t
{
  return stop_loss_loss_count() + exit_signal_loss_count();
}

auto BacktestingSummary::loss_rate() const noexcept -> double
{
  return trade_count() ? static_cast<double>(loss_count()) / trade_count()
                       : 0.0;
}

auto BacktestingSummary::cumulative_losses() const noexcept -> double
{
  return cumulative_stop_loss_losses() + cumulative_exit_signal_losses();
}

auto BacktestingSummary::cumulative_loss_percent() const noexcept -> double
{
  return cumulative_investments()
          ? cumulative_losses() / cumulative_investments() * 100.0
          : 0.0;
}

auto BacktestingSummary::average_loss() const noexcept -> double
{
  return loss_count() ? cumulative_losses() / loss_count() : 0.0;
}

auto BacktestingSummary::break_even_count() const noexcept -> std::size_t
{
  return break_even_count_;
}

void BacktestingSummary::break_even_count(std::size_t count) noexcept
{
  break_even_count_ = count;
}

auto BacktestingSummary::break_even_rate() const noexcept -> double
{
  return trade_count() ? static_cast<double>(break_even_count()) / trade_count()
                       : 0.0;
}

auto BacktestingSummary::profit_factor() const noexcept -> double
{
  if(cumulative_losses() == 0.0) {
    return std::numeric_limits<double>::infinity();
  }
  return cumulative_profits() / -cumulative_losses();
}

auto BacktestingSummary::unrealized_pnl() const noexcept -> double
{
  return trade_session_.is_open() ? trade_session_.pnl() : 0.0;
}

auto BacktestingSummary::unrealized_investment() const noexcept -> double
{
  return trade_session_.is_open() ? trade_session_.investment() : 0.0;
}

auto BacktestingSummary::unrealized_duration() const noexcept -> std::time_t
{
  return trade_session_.is_open() ? trade_session_.duration() : 0;
}

auto BacktestingSummary::equity() const noexcept -> double
{
  const auto partial_realized =
   trade_session_.is_closed() ? 0.0 : trade_session_.realized_pnl();

  return capital() + partial_realized + trade_session_.unrealized_pnl();
}

auto BacktestingSummary::initial_capital() const noexcept -> double
{
  return capital_ - cumulative_pnls();
}

auto BacktestingSummary::drawdown() const noexcept -> double
{
  return peak_equity_ ? (peak_equity_ - equity()) / peak_equity_ * 100.0 : 0.0;
}

} // namespace pludux::backtest
