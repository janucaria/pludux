#include <algorithm>
#include <cstddef>
#include <ctime>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

#include <pludux/backtest/trade_session.hpp>

namespace pludux::backtest {

TradeSession::TradeSession()
: TradeSession{NAN, NAN, NAN, {}}
{
}

TradeSession::TradeSession(double take_profit_price,
                           double stop_loss_price,
                           double trailing_stop_price,
                           std::vector<TradeRecord> trade_records)
: take_profit_price_(take_profit_price)
, stop_loss_price_(stop_loss_price)
, trailing_stop_price_(trailing_stop_price)
, trade_records_(std::move(trade_records))
{
}

auto TradeSession::take_profit_price() const noexcept -> double
{
  return take_profit_price_;
}

void TradeSession::take_profit_price(double price) noexcept
{
  take_profit_price_ = price;
  if(is_open()) {
    trade_records_.back().take_profit_price(price);
  }
}

auto TradeSession::stop_loss_price() const noexcept -> double
{
  return stop_loss_price_;
}

void TradeSession::stop_loss_price(double price) noexcept
{
  stop_loss_price_ = price;
  if(is_open()) {
    trade_records_.back().stop_loss_price(price);
  }
}

auto TradeSession::trailing_stop_price() const noexcept -> double
{
  return trailing_stop_price_;
}

void TradeSession::trailing_stop_price(double price) noexcept
{
  trailing_stop_price_ = price;
  if(is_open()) {
    trade_records_.back().trailing_stop_price(price);
  }
}

auto TradeSession::trade_records() const noexcept
 -> const std::vector<TradeRecord>&
{
  return trade_records_;
}

void TradeSession::trade_records(
 std::vector<TradeRecord> trade_records) noexcept
{
  trade_records_ = std::move(trade_records);
}

auto TradeSession::position_size() const noexcept -> double
{
  if(is_flat()) {
    return NAN;
  }

  return trade_records_.back().position_size();
}

auto TradeSession::average_price() const noexcept -> double
{
  if(is_flat()) {
    return NAN;
  }

  return trade_records_.back().average_price();
}

auto TradeSession::position_value() const noexcept -> double
{
  if(is_flat()) {
    return NAN;
  }

  return std::reduce(trade_records_.cbegin(),
                     trade_records_.cend(),
                     0.0,
                     [](double total, const TradeRecord& record) {
                       if(!record.is_scaled_in()) {
                         return total + record.position_value();
                       }
                       return total;
                     });
}

auto TradeSession::exit_price() const noexcept -> double
{
  if(is_flat()) {
    return NAN;
  }

  return trade_records_.back().exit_price();
}

auto TradeSession::entry_index() const noexcept -> std::size_t
{
  if(is_flat()) {
    return 0;
  }

  return trade_records_.front().entry_index();
}

auto TradeSession::exit_index() const noexcept -> std::size_t
{
  if(is_flat()) {
    return 0;
  }

  return trade_records_.back().exit_index();
}

auto TradeSession::entry_timestamp() const noexcept -> std::time_t
{
  if(is_flat()) {
    return {};
  }

  return trade_records_.front().entry_timestamp();
}

auto TradeSession::exit_timestamp() const noexcept -> std::time_t
{
  if(is_flat()) {
    return {};
  }

  return trade_records_.back().exit_timestamp();
}

auto TradeSession::pnl() const noexcept -> double
{
  if(is_flat()) {
    return NAN;
  }

  return std::reduce(trade_records_.cbegin(),
                     trade_records_.cend(),
                     0.0,
                     [](double total, const TradeRecord& record) {
                       return total + record.pnl();
                     });
}

auto TradeSession::duration() const noexcept -> std::time_t
{
  return std::reduce(trade_records_.cbegin(),
                     trade_records_.cend(),
                     std::time_t{},
                     [](std::time_t total, const TradeRecord& record) {
                       return total + record.duration();
                     });
}

void TradeSession::asset_update(std::time_t timestamp,
                                double price,
                                std::size_t index) noexcept
{
  if(is_open()) {
    auto& last_record = trade_records_.back();
    last_record.exit_timestamp(timestamp);
    last_record.exit_price(price);
    last_record.exit_index(index);
  }
}

auto TradeSession::is_flat() const noexcept -> bool
{
  return trade_records_.empty();
}

auto TradeSession::is_open() const noexcept -> bool
{
  return !trade_records_.empty() && trade_records_.back().is_open();
}

auto TradeSession::is_closed() const noexcept -> bool
{
  return !trade_records_.empty() && trade_records_.back().is_closed();
}

auto TradeSession::is_closed_exit_signal() const noexcept -> bool
{
  return !trade_records_.empty() &&
         trade_records_.back().is_closed_exit_signal();
}

auto TradeSession::is_closed_take_profit() const noexcept -> bool
{
  return !trade_records_.empty() &&
         trade_records_.back().is_closed_take_profit();
}

auto TradeSession::is_closed_stop_loss() const noexcept -> bool
{
  return !trade_records_.empty() && trade_records_.back().is_closed_stop_loss();
}

auto TradeSession::is_summary_session(std::size_t last_index) const noexcept
 -> bool
{
  return is_closed() || (is_open() && exit_index() == last_index);
}

auto TradeSession::at_entry() const noexcept -> bool
{
  if(is_flat()) {
    return false;
  }

  const auto& last_record = trade_records_.back();
  return last_record.is_open() &&
         last_record.entry_index() == last_record.exit_index();
}

} // namespace pludux::backtest
