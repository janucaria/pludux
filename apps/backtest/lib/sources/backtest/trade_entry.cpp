#include <cmath>

#include <pludux/backtest/trade_entry.hpp>

namespace pludux::backtest {

TradeEntry::TradeEntry(double position_size, double price)
: TradeEntry{position_size, price, NAN, false, NAN}
{
}

TradeEntry::TradeEntry(double position_size,
                       double price,
                       double stop_loss_price,
                       bool stop_loss_trailing_enabled,
                       double take_profit_price)
: position_size_(position_size)
, price_(price)
, stop_loss_price_(stop_loss_price)
, take_profit_price_(take_profit_price)
, stop_loss_trailing_enabled_(stop_loss_trailing_enabled)
{
}

auto TradeEntry::position_size() const noexcept -> double
{
  return position_size_;
}

auto TradeEntry::price() const noexcept -> double
{
  return price_;
}

auto TradeEntry::stop_loss_price() const noexcept -> double
{
  return stop_loss_price_;
}

auto TradeEntry::stop_loss_trailing_enabled() const noexcept -> bool
{
  return stop_loss_trailing_enabled_;
}

auto TradeEntry::stop_loss_trailing_price() const noexcept -> double
{
  return stop_loss_trailing_enabled_ ? stop_loss_price_ : NAN;
}

auto TradeEntry::take_profit_price() const noexcept -> double
{
  return take_profit_price_;
}

auto TradeEntry::is_long_direction() const noexcept -> bool
{
  return position_size_ > 0.0;
}

auto TradeEntry::is_short_direction() const noexcept -> bool
{
  return position_size_ < 0.0;
}

} // namespace pludux::backtest
