#ifndef PLUDUX_PLUDUX_BACKTEST_TRADE_RECORD_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADE_RECORD_HPP

#include <cstddef>
#include <ctime>

namespace pludux::backtest {

class TradeRecord {
public:
  enum class Status {
    open,
    closed_exit_signal,
    closed_take_profit,
    closed_stop_loss,
    scaled_in,
    scaled_out
  };

  TradeRecord(Status status,
              double position_size,
              std::size_t entry_index,
              std::size_t market_index,
              std::time_t entry_timestamp,
              std::time_t market_timestamp,
              double entry_price,
              double average_price,
              double exit_price,
              double market_price,
              double stop_loss_price,
              double trailing_stop_price,
              double take_profit_price);

  auto status() const noexcept -> Status;

  void status(Status status) noexcept;

  auto position_size() const noexcept -> double;

  void position_size(double size) noexcept;

  auto entry_index() const noexcept -> std::size_t;

  void entry_index(std::size_t index) noexcept;

  auto market_index() const noexcept -> std::size_t;

  void market_index(std::size_t index) noexcept;

  auto exit_index() const noexcept -> std::size_t;

  auto entry_price() const noexcept -> double;

  void entry_price(double price) noexcept;

  auto average_price() const noexcept -> double;

  void average_price(double price) noexcept;

  auto exit_price() const noexcept -> double;

  void exit_price(double price) noexcept;

  auto market_price() const noexcept -> double;

  void market_price(double price) noexcept;

  auto stop_loss_price() const noexcept -> double;

  void stop_loss_price(double price) noexcept;

  auto trailing_stop_price() const noexcept -> double;

  void trailing_stop_price(double price) noexcept;

  auto take_profit_price() const noexcept -> double;

  void take_profit_price(double price) noexcept;

  auto entry_timestamp() const noexcept -> std::time_t;

  void entry_timestamp(std::time_t timestamp) noexcept;

  auto exit_timestamp() const noexcept -> std::time_t;

  auto market_timestamp() const noexcept -> std::time_t;

  void market_timestamp(std::time_t timestamp) noexcept;

  auto entry_value() const noexcept -> double;

  auto exit_value() const noexcept -> double;

  auto investment() const noexcept -> double;

  auto position_value() const noexcept -> double;

  auto pnl() const noexcept -> double;

  auto duration() const noexcept -> std::time_t;

  auto is_open() const noexcept -> bool;

  auto is_closed() const noexcept -> bool;

  auto is_closed_exit_signal() const noexcept -> bool;

  auto is_closed_take_profit() const noexcept -> bool;

  auto is_closed_stop_loss() const noexcept -> bool;

  auto is_scaled() const noexcept -> bool;

  auto is_scaled_in() const noexcept -> bool;

  auto is_scaled_out() const noexcept -> bool;

  auto is_long_position() const noexcept -> bool;

  auto is_short_position() const noexcept -> bool;

private:
  Status status_;
  double position_size_;

  double entry_price_;
  double average_price_;
  double exit_price_;
  double market_price_;

  double stop_loss_price_;
  double trailing_stop_price_;
  double take_profit_price_;

  std::size_t entry_index_;
  std::size_t market_index_;

  std::time_t entry_timestamp_;
  std::time_t market_timestamp_;
};

} // namespace pludux::backtest

#endif