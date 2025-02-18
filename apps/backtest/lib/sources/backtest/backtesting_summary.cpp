#include <numeric>
#include <utility>

#include <pludux/backtest/backtesting_summary.hpp>

namespace pludux::backtest {

BacktestingSummary::BacktestingSummary()
: open_trade_{}
, closed_trades_{}
{
}

void BacktestingSummary::add_trade(TradeRecord trade)
{
  if(trade.is_open()) {
    open_trade_ = trade;
  } else {
    closed_trades_.push_back(trade);
    open_trade_.reset();
  }
}

auto BacktestingSummary::open_trade() const -> const std::optional<TradeRecord>&
{
  return open_trade_;
}

auto BacktestingSummary::closed_trades() const
 -> const std::vector<TradeRecord>&
{
  return closed_trades_;
}

auto BacktestingSummary::total_profit() const -> double
{
  const auto intial_profit = unrealized_profit();

  return std::reduce(closed_trades_.begin(),
                     closed_trades_.end(),
                     intial_profit,
                     [](const auto& total, const auto& trade) {
                       const auto profit = trade.profit();
                       return total + profit;
                     });
}

auto BacktestingSummary::total_duration() const -> std::time_t
{
  auto total = ongoing_trade_duration();
  return std::reduce(closed_trades_.begin(),
                     closed_trades_.end(),
                     total,
                     [](const auto& total, const auto& trade) {
                       const auto duration = trade.duration();
                       return total + duration;
                     });
}

auto BacktestingSummary::total_trades() const -> std::size_t
{
  return closed_trades_.size() + (open_trade_.has_value() ? 1 : 0);
}

auto BacktestingSummary::average_duration() const -> std::time_t
{
  if(closed_trades_.empty()) {
    return 0;
  }

  return total_duration() / closed_trades_.size();
}

auto BacktestingSummary::unrealized_profit() const -> double
{
  return open_trade_.has_value() ? open_trade_->profit() : 0.0;
}

auto BacktestingSummary::ongoing_trade_duration() const -> std::time_t
{
  return open_trade_.has_value() ? open_trade_->duration() : 0;
}

auto BacktestingSummary::take_profit_rate() const -> double
{
  if(closed_trades_.empty()) {
    return 0;
  }

  auto take_profit = 0;
  for(const auto& trade : closed_trades_) {
    if(trade.is_closed_take_profit()) {
      take_profit++;
    }
  }
  return static_cast<double>(take_profit) / closed_trades_.size();
}

auto BacktestingSummary::average_take_profit() const -> double
{
  auto total = 0.0;
  auto take_profit = 0;
  for(const auto& trade : closed_trades_) {
    if(trade.is_closed_take_profit()) {
      total += trade.profit();
      take_profit++;
    }
  }
  return take_profit ? total / take_profit : 0.0;
}

auto BacktestingSummary::take_profit_expected_value() const -> double
{
  return average_take_profit() * take_profit_rate();
}

auto BacktestingSummary::stop_loss_rate() const -> double
{
  if(closed_trades_.empty()) {
    return 0;
  }

  auto stop_loss = 0;
  for(const auto& trade : closed_trades_) {
    if(trade.is_closed_stop_loss()) {
      stop_loss++;
    }
  }
  return static_cast<double>(stop_loss) / closed_trades_.size();
}

auto BacktestingSummary::average_stop_loss() const -> double
{
  auto total = 0.0;
  auto stop_loss = 0;
  for(const auto& trade : closed_trades_) {
    if(trade.is_closed_stop_loss()) {
      total += trade.profit();
      stop_loss++;
    }
  }
  return stop_loss ? total / stop_loss : 0.0;
}

auto BacktestingSummary::stop_loss_expected_value() const -> double
{
  return average_stop_loss() * stop_loss_rate();
}

auto BacktestingSummary::exit_signal_rate() const -> double
{
  if(closed_trades_.empty()) {
    return 0;
  }

  auto exit_signal = 0;
  for(const auto& trade : closed_trades_) {
    if(trade.is_closed_exit_signal()) {
      exit_signal++;
    }
  }
  return static_cast<double>(exit_signal) / closed_trades_.size();
}

auto BacktestingSummary::average_exit_signal() const -> double
{
  auto total = 0.0;
  auto exit_signal = 0;
  for(const auto& trade : closed_trades_) {
    if(trade.is_closed_exit_signal()) {
      total += trade.profit();
      exit_signal++;
    }
  }
  return exit_signal ? total / exit_signal : 0.0;
}

auto BacktestingSummary::exit_signal_expected_value() const -> double
{
  return average_exit_signal() * exit_signal_rate();
}

auto BacktestingSummary::expected_value() const -> double
{
  return take_profit_expected_value() + stop_loss_expected_value() +
         exit_signal_expected_value();
}

auto BacktestingSummary::win_rate() const -> double
{
  if(closed_trades_.empty()) {
    return 0;
  }

  auto win = 0;
  for(const auto& trade : closed_trades_) {
    if(trade.profit() > 0) {
      win++;
    }
  }
  return static_cast<double>(win) / closed_trades_.size();
}

auto BacktestingSummary::loss_rate() const -> double
{
  if(closed_trades_.empty()) {
    return 0;
  }

  auto loss = 0;
  for(const auto& trade : closed_trades_) {
    if(trade.profit() < 0) {
      loss++;
    }
  }

  return static_cast<double>(loss) / closed_trades_.size();
}

auto BacktestingSummary::break_even_rate() const -> double
{
  if(closed_trades_.empty()) {
    return 0;
  }

  auto break_even = 0;
  for(const auto& trade : closed_trades_) {
    if(trade.profit() == 0) {
      break_even++;
    }
  }
  return static_cast<double>(break_even) / closed_trades_.size();
}

auto BacktestingSummary::average_win() const -> double
{
  auto total = 0.0;
  auto win = 0;
  for(const auto& trade : closed_trades_) {
    if(trade.profit() > 0) {
      total += trade.profit();
      win++;
    }
  }
  return win ? total / win : 0.0;
}

auto BacktestingSummary::average_loss() const -> double
{
  auto total = 0.0;
  auto loss = 0;
  for(const auto& trade : closed_trades_) {
    if(trade.profit() < 0) {
      total += trade.profit();
      loss++;
    }
  }
  return loss ? total / loss : 0.0;
}

} // namespace pludux::backtest
