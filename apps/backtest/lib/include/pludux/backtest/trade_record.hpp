#ifndef PLUDUX_PLUDUX_BACKTEST_TRADE_RECORD_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADE_RECORD_HPP

#include <cstddef>
#include <ctime>

namespace pludux::backtest {

class TradeRecord {
public:
  enum class Status {
    flat,
    open,
    closed_exit_signal,
    closed_take_profit,
    closed_stop_loss
  };

  TradeRecord(Status status,
              double order_size,
              std::size_t entry_index,
              std::size_t at_index,
              std::time_t entry_timestamp,
              std::time_t exit_timestamp,
              double entry_price,
              double exit_price,
              double stop_loss_price,
              double trailing_stop_price,
              double take_profit_price);

  auto order_size() const noexcept -> double;

  void order_size(double size) noexcept;

  auto entry_index() const noexcept -> std::size_t;

  void entry_index(std::size_t index) noexcept;

  auto at_index() const noexcept -> std::size_t;

  void at_index(std::size_t index) noexcept;

  auto entry_price() const noexcept -> double;

  void entry_price(double price) noexcept;

  auto exit_price() const noexcept -> double;

  void exit_price(double price) noexcept;

  auto stop_loss_price() const noexcept -> double;

  void stop_loss_price(double price) noexcept;

  auto trailing_stop_price() const noexcept -> double;

  void trailing_stop_price(double price) noexcept;

  auto take_profit_price() const noexcept -> double;

  void take_profit_price(double price) noexcept;

  auto entry_timestamp() const noexcept -> std::time_t;

  void entry_timestamp(std::time_t timestamp) noexcept;

  auto exit_timestamp() const noexcept -> std::time_t;

  void exit_timestamp(std::time_t timestamp) noexcept;

  auto position_size() const noexcept -> double;

  auto profit() const noexcept -> double;

  auto duration() const noexcept -> std::time_t;

  auto status() const noexcept -> Status;

  void status(Status status) noexcept;

  auto is_flat() const noexcept -> bool;

  auto is_open() const noexcept -> bool;

  auto is_closed() const noexcept -> bool;

  auto is_closed_exit_signal() const noexcept -> bool;

  auto is_closed_take_profit() const noexcept -> bool;

  auto is_closed_stop_loss() const noexcept -> bool;

  auto is_summary_session(std::size_t last_index = 0) const noexcept -> bool;

private:
  double order_size_;

  double entry_price_;
  double exit_price_;

  double stop_loss_price_;
  double trailing_stop_price_;
  double take_profit_price_;

  std::size_t entry_index_;
  std::size_t at_index_;

  std::time_t entry_timestamp_;
  std::time_t exit_timestamp_;

  Status status_;
};

} // namespace pludux::backtest

#endif
