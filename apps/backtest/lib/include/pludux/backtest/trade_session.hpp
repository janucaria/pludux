#ifndef PLUDUX_PLUDUX_BACKTEST_TRADE_SESSION_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADE_SESSION_HPP

#include <cmath>
#include <cstddef>
#include <ctime>
#include <stdexcept>
#include <vector>

#include "./trade_action.hpp"
#include "./trade_record.hpp"

namespace pludux::backtest {

class TradeSession {
public:
  TradeSession();

  TradeSession(double take_profit_price,
               double stop_loss_price,
               double trailing_stop_price,
               std::vector<TradeRecord> trade_records);

  auto take_profit_price() const noexcept -> double;

  void take_profit_price(double price) noexcept;

  auto stop_loss_price() const noexcept -> double;

  void stop_loss_price(double price) noexcept;

  auto trailing_stop_price() const noexcept -> double;

  void trailing_stop_price(double price) noexcept;

  auto trade_records() const noexcept -> const std::vector<TradeRecord>&;

  void trade_records(std::vector<TradeRecord> trade_records) noexcept;

  template<typename... TArgs>
  void emplace_action(TArgs&&... args)
  {
    auto action = TradeAction(std::forward<TArgs>(args)...);

    if(trade_records_.empty()) {
      if(!action.is_buy()) {
        throw std::runtime_error(
         "Cannot add non-buy action when no trade records exist.");
      }

      trade_records_.emplace_back(TradeRecord::Status::open,
                                  action.position_size(),
                                  action.index(),
                                  action.index(),
                                  action.timestamp(),
                                  action.timestamp(),
                                  action.price(),
                                  action.price(),
                                  action.price(),
                                  NAN,
                                  NAN,
                                  NAN);

      return;
    }

    auto& last_record = trade_records_.back();

    if(last_record.is_closed()) {
      throw std::runtime_error("Cannot add action to a closed trade.");
    }

    if(action.is_buy()) {
      const auto last_position_size = last_record.position_size();
      const auto last_position_value = last_record.position_value();

      const auto new_position_size =
       last_position_size + action.position_size();
      const auto new_average_price =
       (last_position_value + action.position_size() * action.price()) /
       new_position_size;

      last_record.status(TradeRecord::Status::scaled_in);
      last_record.exit_index(action.index());
      last_record.exit_timestamp(action.timestamp());
      last_record.exit_price(last_record.average_price());

      const auto position_scale =
       new_position_size / last_record.position_size();

      trade_records_.emplace_back(
       TradeRecord::Status::open,
       new_position_size,
       action.index(),
       action.index(),
       action.timestamp(),
       action.timestamp(),
       action.price(),
       new_average_price,
       action.price(),
       last_record.stop_loss_price() * position_scale,
       last_record.trailing_stop_price() * position_scale,
       last_record.take_profit_price() * position_scale);

    } else {
      if(action.position_size() > last_record.position_size()) {
        throw std::runtime_error(
         "Cannot reduce position size larger than current position size.");
      }

      const auto last_entry_price = last_record.entry_price();
      const auto last_average_price = last_record.average_price();

      const auto remaining_position_size =
       last_record.position_size() - action.position_size();
      const auto position_scale =
       remaining_position_size / last_record.position_size();

      last_record.exit_index(action.index());
      last_record.exit_timestamp(action.timestamp());
      last_record.exit_price(action.price());
      last_record.position_size(action.position_size());

      if(remaining_position_size != 0.0) {
        last_record.status(TradeRecord::Status::scaled_out);

        trade_records_.emplace_back(
         TradeRecord::Status::open,
         remaining_position_size,
         action.index(),
         action.index(),
         action.timestamp(),
         action.timestamp(),
         last_entry_price,
         last_average_price,
         last_average_price,
         last_record.stop_loss_price() * position_scale,
         last_record.trailing_stop_price() * position_scale,
         last_record.take_profit_price() * position_scale);

      } else if(action.is_stop_loss()) {
        last_record.status(TradeRecord::Status::closed_stop_loss);
      } else if(action.is_take_profit()) {
        last_record.status(TradeRecord::Status::closed_take_profit);
      } else {
        last_record.status(TradeRecord::Status::closed_exit_signal);
      }
    }
  }

  auto position_size() const noexcept -> double;

  auto average_price() const noexcept -> double;

  auto position_value() const noexcept -> double;

  auto exit_price() const noexcept -> double;

  auto entry_index() const noexcept -> std::size_t;

  auto exit_index() const noexcept -> std::size_t;

  auto entry_timestamp() const noexcept -> std::time_t;

  auto exit_timestamp() const noexcept -> std::time_t;

  auto pnl() const noexcept -> double;

  auto duration() const noexcept -> std::time_t;

  void
  asset_update(std::time_t timestamp, double price, std::size_t index) noexcept;

  auto is_flat() const noexcept -> bool;

  auto is_open() const noexcept -> bool;

  auto is_closed() const noexcept -> bool;

  auto is_closed_exit_signal() const noexcept -> bool;

  auto is_closed_take_profit() const noexcept -> bool;

  auto is_closed_stop_loss() const noexcept -> bool;

  auto is_summary_session(std::size_t last_index = 0) const noexcept -> bool;

  auto at_entry() const noexcept -> bool;

private:
  double take_profit_price_;
  double stop_loss_price_;
  double trailing_stop_price_;

  std::vector<TradeRecord> trade_records_;
};

} // namespace pludux::backtest

#endif
