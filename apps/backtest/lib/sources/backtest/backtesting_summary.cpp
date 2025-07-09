#include <numeric>
#include <utility>

#include <pludux/backtest/backtesting_summary.hpp>

namespace pludux::backtest {

auto BacktestingSummary::get_next_summary(
 TradeRecord trade_record) const noexcept -> BacktestingSummary
{
  auto summary = *this;

  if(trade_record.is_closed()) {
    const auto pnl = trade_record.pnl();

    summary.sum_of_durations_ += trade_record.duration();
    summary.cumulative_investments_ += trade_record.position_value();

    if(trade_record.is_closed_take_profit()) {
      summary.take_profit_count_++;
      summary.cumulative_take_profits_ += pnl;
    } else if(trade_record.is_closed_stop_loss()) {
      if(pnl > 0) {
        summary.stop_loss_profit_count_++;
        summary.cumulative_stop_loss_profits_ += pnl;
      } else if(pnl < 0) {
        summary.stop_loss_loss_count_++;
        summary.cumulative_stop_loss_losses_ += pnl;
      } else {
        summary.break_even_count_++;
      }
    } else if(trade_record.is_closed_exit_signal()) {
      if(pnl > 0) {
        summary.exit_signal_profit_count_++;
        summary.cumulative_exit_signal_profits_ += pnl;
      } else if(pnl < 0) {
        summary.exit_signal_loss_count_++;
        summary.cumulative_exit_signal_losses_ += pnl;
      } else {
        summary.break_even_count_++;
      }
    }
  }

  summary.trade_record_ = std::move(trade_record);
  return summary;
}

auto BacktestingSummary::trade_record() const noexcept -> const TradeRecord&
{
  return trade_record_;
}

void BacktestingSummary::trade_record(TradeRecord trade_record) noexcept
{
  trade_record_ = std::move(trade_record);
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
  return trade_record_.is_open() ? 1 : 0;
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

auto BacktestingSummary::total_profits() const noexcept -> double
{
  return cumulative_take_profits() + cumulative_stop_loss_profits() +
         cumulative_exit_signal_profits();
}

auto BacktestingSummary::total_profit_percent() const noexcept -> double
{
  return cumulative_investments() ? total_profits() / cumulative_investments()
                                  : 0.0;
}

auto BacktestingSummary::average_profit() const noexcept -> double
{
  return profit_count() ? total_profits() / profit_count() : 0.0;
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

auto BacktestingSummary::total_losses() const noexcept -> double
{
  return cumulative_stop_loss_losses() + cumulative_exit_signal_losses();
}

auto BacktestingSummary::total_loss_percent() const noexcept -> double
{
  return cumulative_investments() ? total_losses() / cumulative_investments()
                                  : 0.0;
}

auto BacktestingSummary::average_loss() const noexcept -> double
{
  return loss_count() ? total_losses() / loss_count() : 0.0;
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
  if(total_losses() == 0.0) {
    return std::numeric_limits<double>::infinity();
  }
  return total_profits() / -total_losses();
}

auto BacktestingSummary::unrealized_pnl() const noexcept -> double
{
  return trade_record_.is_open() ? trade_record_.pnl() : 0.0;
}

auto BacktestingSummary::ongoing_trade_duration() const noexcept -> std::time_t
{
  return trade_record_.is_open() ? trade_record_.duration() : 0;
}

} // namespace pludux::backtest
