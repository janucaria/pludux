#ifndef PLUDUX_PLUDUX_BACKTEST_TRADE_EXIT_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADE_EXIT_HPP

#include <cstddef>
#include <ctime>

namespace pludux::backtest {

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

} // namespace pludux::backtest

#endif
