#ifndef PLUDUX_PLUDUX_BACKTEST_TRADE_ACTION_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADE_ACTION_HPP

#include <cstddef>
#include <ctime>

namespace pludux::backtest {

class TradeAction {
public:
  enum class ActionType { buy, sell, stop_loss, take_profit };

  TradeAction(ActionType action_type,
              double position_size,
              std::time_t timestamp,
              double price,
              std::size_t index);

  auto action_type() const noexcept -> ActionType;

  void action_type(ActionType action_type) noexcept;

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

  auto is_stop_loss() const noexcept -> bool;

  auto is_take_profit() const noexcept -> bool;

  auto is_closed() const noexcept -> bool;

private:
  ActionType action_type_;

  double position_size_;

  std::time_t timestamp_;

  double price_;

  std::size_t index_;
};

} // namespace pludux::backtest

#endif
