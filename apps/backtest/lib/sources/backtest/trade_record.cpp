#include <pludux/backtest/trade_record.hpp>

namespace pludux::backtest {

TradeRecord::TradeRecord(Status status,
                         double order_size,
                         std::size_t entry_index,
                         std::size_t exit_index,
                         double entry_price,
                         double exit_price,
                         double stop_loss_price,
                         double take_profit_price,
                         std::time_t entry_timestamp,
                         std::time_t exit_timestamp)
: order_size_{order_size}
, entry_price_{entry_price}
, exit_price_{exit_price}
, stop_loss_price_{stop_loss_price}
, take_profit_price_{take_profit_price}
, entry_index_{entry_index}
, exit_index_{exit_index}
, entry_timestamp_{entry_timestamp}
, exit_timestamp_{exit_timestamp}
, status_{status}
{
}

auto TradeRecord::order_size() const noexcept -> double
{
  return order_size_;
}

auto TradeRecord::entry_index() const noexcept -> std::size_t
{
  return entry_index_;
}

auto TradeRecord::exit_index() const noexcept -> std::size_t
{
  return exit_index_;
}

auto TradeRecord::entry_price() const noexcept -> double
{
  return entry_price_;
}

auto TradeRecord::exit_price() const noexcept -> double
{
  return exit_price_;
}

auto TradeRecord::entry_timestamp() const noexcept -> std::time_t
{
  return entry_timestamp_;
}

auto TradeRecord::exit_timestamp() const noexcept -> std::time_t
{
  return exit_timestamp_;
}

auto TradeRecord::stop_loss_price() const noexcept -> double
{
  return stop_loss_price_;
}

auto TradeRecord::take_profit_price() const noexcept -> double
{
  return take_profit_price_;
}

auto TradeRecord::position_size() const noexcept -> double
{
  return order_size_ * entry_price_;
}

auto TradeRecord::profit() const noexcept -> double
{
  return (exit_price_ - entry_price_) / entry_price_ * order_size_ * entry_price_;
}

auto TradeRecord::duration() const noexcept -> std::time_t
{
  return exit_timestamp_ - entry_timestamp_;
}

auto TradeRecord::status() const noexcept -> Status
{
  return status_;
}

auto TradeRecord::is_open() const noexcept -> bool
{
  return status_ == Status::open;
}

auto TradeRecord::is_closed() const noexcept -> bool
{
  return !is_open();
}

auto TradeRecord::is_closed_exit_signal() const noexcept -> bool
{
  return status_ == Status::closed_exit_signal;
}

auto TradeRecord::is_closed_take_profit() const noexcept -> bool
{
  return status_ == Status::closed_take_profit;
}

auto TradeRecord::is_closed_stop_loss() const noexcept -> bool
{
  return status_ == Status::closed_stop_loss;
}

} // namespace pludux::backtest
