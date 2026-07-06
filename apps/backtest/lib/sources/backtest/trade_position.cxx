module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <stdexcept>

export module pludux.backtest:trade_position;

import :trade_record;

export namespace pludux::backtest {

class TradePosition {
public:
  TradePosition()
  : TradePosition{0.0, 0, std::time_t{}, 0.0, 0.0, 0.0, 0.0, false, 0.0}
  {
  }

  TradePosition(double position_size,
                std::time_t entry_timestamp,
                double entry_price,
                double total_entry_fees,
                double stop_price,
                double target_price,
                double stop_loss_price,
                bool stop_loss_trailing_enabled,
                double take_profit_price)
  : TradePosition{position_size,
                  entry_price * position_size + total_entry_fees,
                  entry_timestamp,
                  entry_price,
                  total_entry_fees,
                  stop_price,
                  target_price,
                  stop_loss_price,
                  stop_loss_trailing_enabled,
                  take_profit_price}
  {
  }

  TradePosition(double position_size,
                double investment,
                std::time_t entry_timestamp,
                double entry_price,
                double total_entry_fees,
                double stop_price,
                double target_price,
                double stop_loss_price,
                bool stop_loss_trailing_enabled,
                double take_profit_price)
  : position_size_{position_size}
  , investment_{investment}
  , entry_price_{entry_price}
  , total_entry_fees_{total_entry_fees}
  , stop_price_{stop_price}
  , target_price_{target_price}
  , stop_loss_price_{stop_loss_price}
  , take_profit_price_{take_profit_price}
  , stop_loss_trailing_enabled_{stop_loss_trailing_enabled}
  , entry_timestamp_{entry_timestamp}
  {
  }

  auto operator==(const TradePosition&) const noexcept -> bool = default;

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

  auto target_price(this const TradePosition& self) noexcept -> double
  {
    return self.target_price_;
  }

  void target_price(this TradePosition& self, double price) noexcept
  {
    self.target_price_ = price;
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

  auto take_profit_price(this const TradePosition& self) noexcept -> double
  {
    return self.take_profit_price_;
  }

  void take_profit_price(this TradePosition& self, double price) noexcept
  {
    self.take_profit_price_ = price;
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
    return self.investment() / self.position_size();
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

  auto scaled_in(this TradePosition& self,
                 double action_position_size,
                 std::time_t action_timestamp,
                 double action_price,
                 double action_total_fees,
                 double stop_price,
                 double target_price,
                 double stop_loss_price,
                 bool stop_loss_trailing_enabled,
                 double take_profit_price) -> TradeRecord
  {
    if(self.is_closed()) {
      throw std::runtime_error("Cannot scaled in to a closed trade.");
    }

    const auto scaled_in_record = TradeRecord{TradeRecord::Status::scaled_in,
                                              self.position_size(),
                                              self.investment(),

                                              self.entry_timestamp(),
                                              self.entry_price(),
                                              self.total_entry_fees(),

                                              action_timestamp,
                                              action_price,
                                              action_total_fees,

                                              self.stop_price(),
                                              self.target_price(),
                                              self.stop_loss_price(),
                                              self.take_profit_price()};

    const auto last_position_size = self.position_size();
    const auto new_investment =
     action_position_size * action_price + action_total_fees;

    const auto updated_position_size =
     last_position_size + action_position_size;
    const auto updated_investment = self.investment() + new_investment;

    self.position_size(updated_position_size);
    self.investment(updated_investment);

    self.stop_price(stop_price);
    self.target_price(target_price);
    self.stop_loss_price(stop_loss_price);
    self.stop_loss_trailing_enabled(stop_loss_trailing_enabled);
    self.take_profit_price(take_profit_price);

    return scaled_in_record;
  }

  auto scaled_out(this TradePosition& self,
                  double action_position_size,
                  std::time_t action_timestamp,
                  double action_price,
                  double action_total_fees,
                  TradeRecord::Status trade_status) -> TradeRecord
  {
    if(self.is_closed()) {
      throw std::runtime_error("Cannot add action to a closed trade.");
    }

    const auto last_position_size = self.position_size();
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

    auto exit_record = TradeRecord{trade_status,
                                   action_position_size,
                                   investment_closed,
                                   self.entry_timestamp(),
                                   self.entry_price(),
                                   self.total_entry_fees(),
                                   action_timestamp,
                                   action_price,
                                   action_total_fees,
                                   self.stop_price(),
                                   self.target_price(),
                                   self.stop_loss_price(),
                                   self.take_profit_price()};

    const auto updated_investment = self.investment() - investment_closed;
    self.investment(updated_investment);

    self.position_size(remaining_position_size);

    return exit_record;
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
                                double high,
                                double low) noexcept -> bool
  {
    const auto reference_price = self.take_profit_price();

    if(self.is_closed() || std::isnan(reference_price)) {
      return false;
    }

    return self.is_long_direction() ? reference_price < high
                                    : reference_price > low;
  }

private:
  double position_size_;
  double investment_;

  double entry_price_;
  double total_entry_fees_;

  double stop_price_;
  double target_price_;
  double stop_loss_price_;
  double take_profit_price_;
  bool stop_loss_trailing_enabled_;

  std::time_t entry_timestamp_;
};

} // namespace pludux::backtest
