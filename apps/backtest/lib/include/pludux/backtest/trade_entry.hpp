#ifndef PLUDUX_PLUDUX_BACKTEST_TRADE_ENTRY_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADE_ENTRY_HPP

#include <cstddef>
#include <ctime>

namespace pludux::backtest {

class TradeEntry {
public:
  TradeEntry(double position_size, double price);

  TradeEntry(double position_size,
             double price,
             double stop_loss_price,
             bool stop_loss_trailing_enabled,
             double take_profit_price);

  auto position_size() const noexcept -> double;

  auto price() const noexcept -> double;

  auto stop_loss_price() const noexcept -> double;

  auto stop_loss_trailing_enabled() const noexcept -> bool;

  auto stop_loss_trailing_price() const noexcept -> double;

  auto take_profit_price() const noexcept -> double;

  auto is_long_direction() const noexcept -> bool;

  auto is_short_direction() const noexcept -> bool;

private:
  double position_size_;
  double price_;

  double stop_loss_price_;
  double take_profit_price_;

  bool stop_loss_trailing_enabled_;
};

} // namespace pludux::backtest

#endif
