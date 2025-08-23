#include <pludux/backtest/trade_action.hpp>

namespace pludux::backtest {

TradeAction::TradeAction(double position_size,
                         std::time_t timestamp,
                         double price,
                         std::size_t index,
                         Reason reason)
: position_size_{position_size}
, timestamp_{timestamp}
, price_{price}
, index_{index}
, reason_{reason}
{
}

auto TradeAction::reason() const noexcept -> Reason
{
  return reason_;
}

void TradeAction::reason(Reason reason) noexcept
{
  reason_ = reason;
}

auto TradeAction::price() const noexcept -> double
{
  return price_;
}

void TradeAction::price(double price) noexcept
{
  price_ = price;
}

auto TradeAction::index() const noexcept -> std::size_t
{
  return index_;
}

void TradeAction::index(std::size_t index) noexcept
{
  index_ = index;
}

auto TradeAction::timestamp() const noexcept -> std::time_t
{
  return timestamp_;
}

void TradeAction::timestamp(std::time_t timestamp) noexcept
{
  timestamp_ = timestamp;
}

auto TradeAction::position_size() const noexcept -> double
{
  return position_size_;
}

void TradeAction::position_size(double size) noexcept
{
  position_size_ = size;
}

auto TradeAction::is_buy() const noexcept -> bool
{
  return position_size() > 0;
}

auto TradeAction::is_sell() const noexcept -> bool
{
  return position_size() < 0;
}

auto TradeAction::is_reason_entry() const noexcept -> bool
{
  return reason_ == Reason::entry;
}

auto TradeAction::is_reason_exit() const noexcept -> bool
{
  return reason_ == Reason::exit;
}

auto TradeAction::is_reason_stop_loss() const noexcept -> bool
{
  return reason_ == Reason::stop_loss;
}

auto TradeAction::is_reason_take_profit() const noexcept -> bool
{
  return reason_ == Reason::take_profit;
}

} // namespace pludux::backtest
