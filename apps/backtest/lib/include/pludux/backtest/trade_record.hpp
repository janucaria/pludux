#ifndef PLUDUX_PLUDUX_BACKTEST_TRADE_RECORD_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADE_RECORD_HPP

#include <ctime>

namespace pludux::backtest {

class TradeRecord {
public:
  enum class Status {
    open,
    closed_exit_signal,
    closed_take_profit,
    closed_stop_loss
  };

  TradeRecord(Status status,
              double order_size,
              std::size_t entry_index,
              std::size_t exit_index,
              double entry_price,
              double exit_price,
              double stop_loss_price,
              double take_profit_price,
              std::time_t entry_timestamp,
              std::time_t exit_timestamp);

  auto order_size() const noexcept -> double;

  auto entry_index() const noexcept -> std::size_t;

  auto exit_index() const noexcept -> std::size_t;

  auto entry_price() const noexcept -> double;

  auto exit_price() const noexcept -> double;

  auto stop_loss_price() const noexcept -> double;

  auto take_profit_price() const noexcept -> double;

  auto entry_timestamp() const noexcept -> std::time_t;

  auto exit_timestamp() const noexcept -> std::time_t;

  auto position_size() const noexcept -> double;

  auto profit() const noexcept -> double;

  auto duration() const noexcept -> std::time_t;

  auto status() const noexcept -> Status;

  auto is_open() const noexcept -> bool;

  auto is_closed() const noexcept -> bool;

  auto is_closed_exit_signal() const noexcept -> bool;

  auto is_closed_take_profit() const noexcept -> bool;

  auto is_closed_stop_loss() const noexcept -> bool;

private:
  double order_size_;

  double entry_price_;
  double exit_price_;

  double stop_loss_price_;
  double take_profit_price_;

  std::size_t entry_index_;
  std::size_t exit_index_;

  std::time_t entry_timestamp_;
  std::time_t exit_timestamp_;

  Status status_;
};

} // namespace pludux::backtest

#endif