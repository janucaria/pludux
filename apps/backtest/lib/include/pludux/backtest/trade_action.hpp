#ifndef PLUDUX_PLUDUX_BACKTEST_TRADE_ACTION_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADE_ACTION_HPP

#include <cstddef>
#include <ctime>

namespace pludux::backtest {

class TradeAction {
public:
  enum class Reason { entry, exit, stop_loss, take_profit };

  TradeAction(double position_size,
              std::time_t timestamp,
              double price,
              std::size_t index,
              Reason reason);

  auto reason() const noexcept -> Reason;

  void reason(Reason reason) noexcept;

  auto price() const noexcept -> double;

  void price(double price) noexcept;

  auto index() const noexcept -> std::size_t;

  void index(std::size_t index) noexcept;

  auto timestamp() const noexcept -> std::time_t;

  void timestamp(std::time_t timestamp) noexcept;

  auto position_size() const noexcept -> double;

  void position_size(double size) noexcept;

  auto is_buy() const noexcept -> bool;

  auto is_sell() const noexcept -> bool;

  auto is_reason_entry() const noexcept -> bool;

  auto is_reason_exit() const noexcept -> bool;

  auto is_reason_stop_loss() const noexcept -> bool;

  auto is_reason_take_profit() const noexcept -> bool;

private:
  double position_size_;

  std::time_t timestamp_;

  double price_;

  std::size_t index_;

  Reason reason_;
};

} // namespace pludux::backtest

#endif
