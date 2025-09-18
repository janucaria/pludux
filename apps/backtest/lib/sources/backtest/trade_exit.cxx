module;

#include <cstddef>
#include <ctime>

export module pludux.backtest:trade_exit;

export namespace pludux::backtest {

class TradeExit {
public:
  enum class Reason { signal, stop_loss, take_profit };

  TradeExit(double position_size, double price, Reason reason);

  auto position_size() const noexcept -> double;

  auto price() const noexcept -> double;

  auto reason() const noexcept -> Reason;

  auto is_long_direction() const noexcept -> bool;

  auto is_short_direction() const noexcept -> bool;

private:
  Reason reason_;

  double position_size_;
  double price_;
};

// -------------------------------------------------------------------

TradeExit::TradeExit(double position_size, double price, Reason reason)
: reason_(reason)
, position_size_(position_size)
, price_(price)
{
}

auto TradeExit::position_size() const noexcept -> double
{
  return position_size_;
}

auto TradeExit::price() const noexcept -> double
{
  return price_;
}

auto TradeExit::reason() const noexcept -> Reason
{
  return reason_;
}

auto TradeExit::is_long_direction() const noexcept -> bool
{
  return position_size_ > 0.0;
}

auto TradeExit::is_short_direction() const noexcept -> bool
{
  return position_size_ < 0.0;
}

} // namespace pludux::backtest
