#include <pludux/backtest/trade_action.hpp>

namespace pludux::backtest {

TradeAction::TradeAction(ActionType action_type,
                         double position_size,
                         std::time_t timestamp,
                         double price,
                         std::size_t index)
: action_type_{action_type}
, position_size_{position_size}
, timestamp_{timestamp}
, price_{price}
, index_{index}
{
}

auto TradeAction::action_type() const noexcept -> ActionType
{
  return action_type_;
}

void TradeAction::action_type(ActionType action_type) noexcept
{
  action_type_ = action_type;
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
  return action_type_ == ActionType::buy;
}

auto TradeAction::is_sell() const noexcept -> bool
{
  return action_type_ == ActionType::sell;
}

auto TradeAction::is_stop_loss() const noexcept -> bool
{
  return action_type_ == ActionType::stop_loss;
}

auto TradeAction::is_take_profit() const noexcept -> bool
{
  return action_type_ == ActionType::take_profit;
}

auto TradeAction::is_closed() const noexcept -> bool
{
  return is_sell() || is_stop_loss() || is_take_profit();
}

} // namespace pludux::backtest
