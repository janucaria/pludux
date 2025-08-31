#ifndef PLUDUX_PLUDUX_BACKTEST_TRADE_POSITION_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADE_POSITION_HPP

#include <cmath>
#include <cstddef>
#include <ctime>
#include <vector>

#include <pludux/backtest/trade_record.hpp>

namespace pludux::backtest {

class TradePosition {
public:

  TradePosition(double position_size,
                std::time_t entry_timestamp,
                double entry_price,
                std::size_t entry_index,
                double stop_loss_initial_price,
                double stop_loss_trailing_price,
                double take_profit_price);

  TradePosition(double position_size,
                double average_price,
                std::time_t entry_timestamp,
                double entry_price,
                std::size_t entry_index,
                double stop_loss_initial_price,
                double stop_loss_trailing_price,
                double take_profit_price,
                std::vector<TradeRecord> realized_records);

  auto position_size() const noexcept -> double;

  void position_size(double size) noexcept;

  auto average_price() const noexcept -> double;

  void average_price(double price) noexcept;

  auto entry_price() const noexcept -> double;

  void entry_price(double price) noexcept;

  auto stop_loss_initial_price() const noexcept -> double;

  void stop_loss_initial_price(double price) noexcept;

  auto stop_loss_trailing_price() const noexcept -> double;

  void stop_loss_trailing_price(double price) noexcept;

  auto take_profit_price() const noexcept -> double;

  void take_profit_price(double price) noexcept;

  auto entry_index() const noexcept -> std::size_t;

  void entry_index(std::size_t index) noexcept;

  auto entry_timestamp() const noexcept -> std::time_t;

  void entry_timestamp(std::time_t timestamp) noexcept;

  auto realized_records() const noexcept -> const std::vector<TradeRecord>&;

  void realized_records(std::vector<TradeRecord> records) noexcept;

  auto stop_loss_price() const noexcept -> double;

  auto unrealized_position_size() const noexcept -> double;

  auto unrealized_investment() const noexcept -> double;

  auto unrealized_pnl(double market_price) const noexcept -> double;

  auto unrealized_duration(std::time_t market_timestamp) const noexcept
   -> std::time_t;

  auto realized_position_size() const noexcept -> double;

  auto realized_investment() const noexcept -> double;

  auto realized_pnl() const noexcept -> double;

  auto realized_duration() const noexcept -> std::time_t;

  auto is_closed() const noexcept -> bool;

  auto is_long_direction() const noexcept -> bool;

  auto is_short_direction() const noexcept -> bool;

  void scaled_in(double action_position_size,
                 std::time_t action_timestamp,
                 double action_price,
                 std::size_t action_index);

  void scaled_out(double action_position_size,
                  std::time_t action_timestamp,
                  double action_price,
                  std::size_t action_index,
                  TradeRecord::Status trade_status);

  auto trigger_stop_loss(double prev_close, double high, double low) -> bool;

  auto trigger_take_profit(double high, double low) -> bool;

private:
  double position_size_;
  double average_price_;

  double entry_price_;

  double stop_loss_initial_price_;
  double stop_loss_trailing_price_;
  double take_profit_price_;

  std::size_t entry_index_;
  std::time_t entry_timestamp_;

  std::vector<TradeRecord> realized_records_;
};

} // namespace pludux::backtest

#endif
