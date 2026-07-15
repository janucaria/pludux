module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <stdexcept>
#include <utility>
#include <vector>

export module pludux.backtest:trade_position;

import :closed_trade;
import :open_position_snapshot;
import :trade_event;
import :take_profit_level;
import :signal_exit_state;

export namespace pludux::backtest {

class TradePosition {
public:
  TradePosition()
  : TradePosition{0, 0.0, 0, std::time_t{}, 0.0, 0.0, 0.0, false}
  {
  }

  TradePosition(std::size_t trade_id,
                double position_size,
                std::time_t entry_timestamp,
                double entry_price,
                double total_entry_fees,
                double stop_price,
                double stop_loss_price,
                bool stop_loss_trailing_enabled)
  : TradePosition{trade_id,
                  position_size,
                  entry_price * position_size + total_entry_fees,
                  entry_timestamp,
                  entry_price,
                  total_entry_fees,
                  stop_price,
                  stop_loss_price,
                  stop_loss_trailing_enabled}
  {
  }

  TradePosition(std::size_t trade_id,
                double position_size,
                double investment,
                std::time_t entry_timestamp,
                double entry_price,
                double total_entry_fees,
                double stop_price,
                double stop_loss_price,
                bool stop_loss_trailing_enabled)
  : trade_id_{trade_id}
  , trade_event_count_{1}
  , position_size_{position_size}
  , investment_{investment}
  , entry_price_{entry_price}
  , total_entry_fees_{total_entry_fees}
  , stop_price_{stop_price}
  , stop_loss_price_{stop_loss_price}
  , stop_loss_trailing_enabled_{stop_loss_trailing_enabled}
  , entry_timestamp_{entry_timestamp}
  {
  }

  auto operator==(const TradePosition&) const noexcept -> bool = default;

  auto trade_id(this const TradePosition& self) noexcept -> std::size_t
  {
    return self.trade_id_;
  }

  auto trade_event_count(this const TradePosition& self) noexcept -> std::size_t
  {
    return self.trade_event_count_;
  }

  auto position_size(this const TradePosition& self) noexcept -> double
  {
    return self.position_size_;
  }

  void position_size(this TradePosition& self, double size) noexcept
  {
    self.position_size_ = size;
  }

  auto investment(this const TradePosition& self) noexcept -> double
  {
    return self.investment_;
  }

  void investment(this TradePosition& self, double investment) noexcept
  {
    self.investment_ = investment;
  }

  auto entry_price(this const TradePosition& self) noexcept -> double
  {
    return self.entry_price_;
  }

  void entry_price(this TradePosition& self, double price) noexcept
  {
    self.entry_price_ = price;
  }

  auto total_entry_fees(this const TradePosition& self) noexcept -> double
  {
    return self.total_entry_fees_;
  }

  void total_entry_fees(this TradePosition& self, double fees) noexcept
  {
    self.total_entry_fees_ = fees;
  }

  auto stop_price(this const TradePosition& self) noexcept -> double
  {
    return self.stop_price_;
  }

  void stop_price(this TradePosition& self, double price) noexcept
  {
    self.stop_price_ = price;
  }

  auto stop_loss_price(this const TradePosition& self) noexcept -> double
  {
    return self.stop_loss_price_;
  }

  void stop_loss_price(this TradePosition& self, double price) noexcept
  {
    self.stop_loss_price_ = price;
  }

  auto stop_loss_trailing_enabled(this const TradePosition& self) noexcept
   -> bool
  {
    return self.stop_loss_trailing_enabled_;
  }

  void stop_loss_trailing_enabled(this TradePosition& self,
                                  bool enabled) noexcept
  {
    self.stop_loss_trailing_enabled_ = enabled;
  }

  auto take_profit_levels(this const TradePosition& self) noexcept
   -> const std::vector<TakeProfitLevel>&
  {
    return self.take_profit_levels_;
  }

  auto take_profit_levels(this TradePosition& self) noexcept
   -> std::vector<TakeProfitLevel>&
  {
    return self.take_profit_levels_;
  }

  void take_profit_levels(this TradePosition& self,
                          std::vector<TakeProfitLevel> levels) noexcept
  {
    self.take_profit_levels_ = std::move(levels);
  }

  auto signal_exit_states(this const TradePosition& self) noexcept
   -> const std::vector<SignalExitState>&
  {
    return self.signal_exit_states_;
  }

  auto signal_exit_states(this TradePosition& self) noexcept
   -> std::vector<SignalExitState>&
  {
    return self.signal_exit_states_;
  }

  void signal_exit_states(this TradePosition& self,
                          std::vector<SignalExitState> states) noexcept
  {
    self.signal_exit_states_ = std::move(states);
  }

  auto entry_timestamp(this const TradePosition& self) noexcept -> std::time_t
  {
    return self.entry_timestamp_;
  }

  void entry_timestamp(this TradePosition& self, std::time_t timestamp) noexcept
  {
    self.entry_timestamp_ = timestamp;
  }

  auto average_price(this const TradePosition& self) noexcept -> double
  {
    return self.position_size() ? self.investment() / self.position_size()
                                : 0.0;
  }

  auto unrealized_position_size(this const TradePosition& self) noexcept
   -> double
  {
    return self.position_size();
  }

  auto unrealized_investment(this const TradePosition& self) noexcept -> double
  {
    return self.investment();
  }

  auto unrealized_pnl(this const TradePosition& self,
                      double market_price) noexcept -> double
  {
    return self.unrealized_position_size() *
           (market_price - self.average_price());
  }

  auto unrealized_duration(this const TradePosition& self,
                           std::time_t market_timestamp) noexcept -> std::time_t
  {
    return market_timestamp - self.entry_timestamp();
  }

  auto is_closed(this const TradePosition& self) noexcept -> bool
  {
    return self.position_size() == 0;
  }

  auto is_long_direction(this const TradePosition& self) noexcept -> bool
  {
    return self.position_size() > 0;
  }

  auto is_short_direction(this const TradePosition& self) noexcept -> bool
  {
    return self.position_size() < 0;
  }

  auto snapshot(this const TradePosition& self,
                std::time_t market_timestamp,
                double market_price) noexcept -> OpenPositionSnapshot
  {
    return OpenPositionSnapshot{self.trade_id(),
                                self.entry_timestamp(),
                                market_timestamp,
                                market_price,
                                self.position_size(),
                                self.investment(),
                                self.entry_price(),
                                self.total_entry_fees(),
                                self.stop_price(),
                                self.stop_loss_price(),
                                self.take_profit_levels(),
                                self.signal_exit_states()};
  }

  auto scaled_in(this TradePosition& self,
                 std::size_t event_id,
                 double action_position_size,
                 std::time_t action_timestamp,
                 double action_price,
                 double action_total_fees,
                 double stop_price,
                 double stop_loss_price,
                 bool stop_loss_trailing_enabled) -> TradeEvent
  {
    if(self.is_closed()) {
      throw std::runtime_error("Cannot scaled in to a closed trade.");
    }

    const auto last_position_size = self.position_size();
    const auto last_investment = self.investment();
    const auto last_average_price = self.average_price();
    const auto new_investment =
     action_position_size * action_price + action_total_fees;

    const auto updated_position_size =
     last_position_size + action_position_size;
    const auto updated_investment = self.investment() + new_investment;

    self.position_size(updated_position_size);
    self.investment(updated_investment);

    self.stop_price(stop_price);
    self.stop_loss_price(stop_loss_price);
    self.stop_loss_trailing_enabled(stop_loss_trailing_enabled);
    self.trade_event_count_++;

    return TradeEvent{self.trade_id(),
                      event_id,
                      self.trade_event_count(),
                      TradeEvent::Type::scale_in,
                      action_timestamp,
                      action_price,
                      action_position_size,
                      action_total_fees,
                      last_position_size,
                      last_investment,
                      last_average_price,
                      self.position_size(),
                      self.investment(),
                      self.average_price(),
                      self.stop_price(),
                      self.stop_loss_price(),
                      self.take_profit_levels(),
                      self.signal_exit_states()};
  }

  auto scaled_out(this TradePosition& self,
                  std::size_t event_id,
                  double action_position_size,
                  std::time_t action_timestamp,
                  double action_price,
                  double action_total_fees,
                  TradeEvent::Type event_type) -> TradeEvent
  {
    if(self.is_closed()) {
      throw std::runtime_error("Cannot add action to a closed trade.");
    }

    const auto last_position_size = self.position_size();
    const auto last_investment = self.investment();
    const auto last_average_price = self.average_price();
    const auto remaining_position_size =
     last_position_size - action_position_size;

    if(self.is_long_direction()) {
      if(remaining_position_size < 0) {
        throw std::runtime_error("Cannot reduce position size larger than "
                                 "current long position size.");
      }
    } else if(self.is_short_direction()) {
      if(remaining_position_size > 0) {
        throw std::runtime_error("Cannot reduce position size larger than "
                                 "current short position size.");
      }
    }

    const auto investment_closed = action_position_size * self.average_price();

    const auto updated_investment = self.investment() - investment_closed;
    self.investment(updated_investment);

    self.position_size(remaining_position_size);
    self.trade_event_count_++;

    return TradeEvent{self.trade_id(),
                      event_id,
                      self.trade_event_count(),
                      event_type,
                      action_timestamp,
                      action_price,
                      action_position_size,
                      action_total_fees,
                      last_position_size,
                      last_investment,
                      last_average_price,
                      self.position_size(),
                      self.investment(),
                      self.average_price(),
                      self.stop_price(),
                      self.stop_loss_price(),
                      self.take_profit_levels(),
                      self.signal_exit_states()};
  }

  auto closed_trade(this const TradePosition& self,
                    std::size_t exit_event_id,
                    TradeEvent::Type exit_type,
                    std::time_t exit_timestamp,
                    double exit_price,
                    double total_exit_fees,
                    double closed_position_size,
                    double closed_investment) noexcept -> ClosedTrade
  {
    return ClosedTrade{self.trade_id(),
                       exit_event_id,
                       exit_type,
                       self.entry_timestamp(),
                       exit_timestamp,
                       closed_position_size,
                       closed_investment,
                       self.entry_price(),
                       exit_price,
                       self.total_entry_fees(),
                       total_exit_fees,
                       self.stop_price(),
                       self.stop_loss_price(),
                       self.take_profit_levels(),
                       self.signal_exit_states()};
  }

  void update_trailing_stop(this TradePosition& self,
                            double prev_close) noexcept
  {
    const auto stop_price = self.stop_price();

    if(self.is_closed() || !self.stop_loss_trailing_enabled() ||
       std::isnan(stop_price) || std::isnan(self.stop_loss_price())) {
      return;
    }

    const auto risk = self.average_price() - stop_price;
    const auto new_stop_price = prev_close - risk;

    if(self.is_short_direction()) {
      if(new_stop_price < self.stop_loss_price()) {
        self.stop_loss_price(new_stop_price);
      }

      return;
    }

    if(new_stop_price > self.stop_loss_price()) {
      self.stop_loss_price(new_stop_price);
    }
  }

  auto is_stop_loss_triggered(this const TradePosition& self,
                              double high,
                              double low) noexcept -> bool
  {
    const auto stop_price = self.stop_loss_price();

    if(self.is_closed() || std::isnan(stop_price)) {
      return false;
    }

    return self.is_long_direction() ? low <= stop_price : high >= stop_price;
  }

  auto is_take_profit_triggered(this const TradePosition& self,
                                std::size_t index,
                                double high,
                                double low) noexcept -> bool
  {
    if(self.is_closed() || index >= self.take_profit_levels_.size()) {
      return false;
    }
    const auto& level = self.take_profit_levels_[index];
    if(!level.active()) {
      return false;
    }
    return self.is_long_direction() ? level.price() < high
                                    : level.price() > low;
  }

private:
  std::size_t trade_id_;
  std::size_t trade_event_count_;

  double position_size_;
  double investment_;

  double entry_price_;
  double total_entry_fees_;

  double stop_price_;
  double stop_loss_price_;
  std::vector<TakeProfitLevel> take_profit_levels_;
  std::vector<SignalExitState> signal_exit_states_;
  bool stop_loss_trailing_enabled_;

  std::time_t entry_timestamp_;
};

} // namespace pludux::backtest
