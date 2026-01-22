module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

export module pludux.backtest:trade_position;

import :trade_record;

export namespace pludux::backtest {

class TradePosition {
public:
  TradePosition(double position_size,
                std::time_t entry_timestamp,
                double entry_price,
                double total_entry_fees,
                std::size_t entry_index,
                double stop_loss_initial_price,
                double stop_loss_trailing_price,
                double take_profit_price)
  : TradePosition{position_size,
                  entry_price * position_size + total_entry_fees,
                  entry_timestamp,
                  entry_price,
                  total_entry_fees,
                  entry_index,
                  stop_loss_initial_price,
                  stop_loss_trailing_price,
                  take_profit_price,
                  {}}
  {
  }

  TradePosition(double position_size,
                double investment,
                std::time_t entry_timestamp,
                double entry_price,
                double total_entry_fees,
                std::size_t entry_index,
                double stop_loss_initial_price,
                double stop_loss_trailing_price,
                double take_profit_price,
                std::vector<TradeRecord> realized_records)
  : position_size_{position_size}
  , investment_{investment}
  , entry_price_{entry_price}
  , total_entry_fees_{total_entry_fees}
  , stop_loss_initial_price_{stop_loss_initial_price}
  , stop_loss_trailing_price_{stop_loss_trailing_price}
  , take_profit_price_{take_profit_price}
  , entry_timestamp_{entry_timestamp}
  , entry_index_{entry_index}
  , realized_records_{std::move(realized_records)}
  {
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

  auto stop_loss_initial_price(this const TradePosition& self) noexcept
   -> double
  {
    return self.stop_loss_initial_price_;
  }

  void stop_loss_initial_price(this TradePosition& self, double price) noexcept
  {
    self.stop_loss_initial_price_ = price;
  }

  auto stop_loss_trailing_price(this const TradePosition& self) noexcept
   -> double
  {
    return self.stop_loss_trailing_price_;
  }

  void stop_loss_trailing_price(this TradePosition& self, double price) noexcept
  {
    self.stop_loss_trailing_price_ = price;
  }

  auto take_profit_price(this const TradePosition& self) noexcept -> double
  {
    return self.take_profit_price_;
  }

  void take_profit_price(this TradePosition& self, double price) noexcept
  {
    self.take_profit_price_ = price;
  }

  auto entry_index(this const TradePosition& self) noexcept -> std::size_t
  {
    return self.entry_index_;
  }

  void entry_index(this TradePosition& self, std::size_t index) noexcept
  {
    self.entry_index_ = index;
  }

  auto entry_timestamp(this const TradePosition& self) noexcept -> std::time_t
  {
    return self.entry_timestamp_;
  }

  void entry_timestamp(this TradePosition& self, std::time_t timestamp) noexcept
  {
    self.entry_timestamp_ = timestamp;
  }

  auto realized_records(this const TradePosition& self) noexcept
   -> const std::vector<TradeRecord>&
  {
    return self.realized_records_;
  }

  void realized_records(this TradePosition& self,
                        std::vector<TradeRecord> records) noexcept
  {
    self.realized_records_ = std::move(records);
  }

  auto stop_loss_price(this const TradePosition& self) noexcept -> double
  {
    return std::isnan(self.stop_loss_trailing_price())
            ? self.stop_loss_initial_price()
            : self.stop_loss_trailing_price();
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

  auto realized_position_size(this const TradePosition& self) noexcept -> double
  {
    return std::ranges::fold_left(
     self.realized_records(), 0.0, [](double total, const TradeRecord& record) {
       return total + record.position_size();
     });
  }

  auto realized_investment(this const TradePosition& self) noexcept -> double
  {
    return std::ranges::fold_left(
     self.realized_records(), 0.0, [](double total, const TradeRecord& record) {
       return total + record.investment();
     });
  }

  auto realized_pnl(this const TradePosition& self) noexcept -> double
  {
    return std::ranges::fold_left(
     self.realized_records(), 0.0, [](double total, const TradeRecord& record) {
       return total + record.pnl();
     });
  }

  auto realized_duration(this const TradePosition& self) noexcept -> std::time_t
  {
    return std::ranges::fold_left(
     self.realized_records(),
     std::time_t{},
     [](std::time_t total, const TradeRecord& record) {
       return total + record.duration();
     });
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

  void scaled_in(this TradePosition& self,
                 double action_position_size,
                 std::time_t action_timestamp,
                 double action_price,
                 double action_total_fees,
                 std::size_t action_index)
  {
    if(self.is_closed()) {
      throw std::runtime_error("Cannot scaled in to a closed trade.");
    }

    const auto last_investment = self.investment();

    auto scaled_in_record = TradeRecord{TradeRecord::Status::scaled_in,
                                        0.0,
                                        last_investment,
                                        self.entry_timestamp(),
                                        self.entry_price(),
                                        action_total_fees,
                                        self.entry_index(),
                                        action_timestamp,
                                        action_price,
                                        0.0,
                                        action_index,
                                        self.stop_loss_initial_price(),
                                        self.stop_loss_trailing_price(),
                                        self.take_profit_price()};

    const auto last_position_size = self.position_size();
    const auto new_investment =
     action_position_size * action_price + action_total_fees;

    const auto updated_position_size =
     last_position_size + action_position_size;
    const auto updated_investment = last_investment + new_investment;

    self.position_size(updated_position_size);
    self.investment(updated_investment);

    self.realized_records_.emplace_back(std::move(scaled_in_record));
  }

  void scaled_out(this TradePosition& self,
                  double action_position_size,
                  std::time_t action_timestamp,
                  double action_price,
                  double action_total_fees,
                  std::size_t action_index,
                  TradeRecord::Status trade_status)
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
                                   self.entry_index(),
                                   action_timestamp,
                                   action_price,
                                   action_total_fees,
                                   action_index,
                                   self.stop_loss_initial_price(),
                                   self.stop_loss_trailing_price(),
                                   self.take_profit_price()};
    self.realized_records_.emplace_back(std::move(exit_record));

    const auto updated_investment = self.investment() - investment_closed;
    self.investment(updated_investment);

    self.position_size(remaining_position_size);
  }

  auto trigger_stop_loss(this TradePosition& self,
                         double prev_close,
                         double high,
                         double low) -> bool
  {
    const auto initial_stop_price = self.stop_loss_initial_price();

    if(self.is_closed() || std::isnan(initial_stop_price)) {
      return false;
    }

    const auto is_short_position = self.is_short_direction();
    const auto risk = self.average_price() - initial_stop_price;
    const auto new_stop_price = prev_close - risk;

    if(is_short_position) {
      if(new_stop_price < self.stop_loss_trailing_price()) {
        self.stop_loss_trailing_price(new_stop_price);
      }

      return high >= self.stop_loss_price();
    }

    if(new_stop_price > self.stop_loss_trailing_price()) {
      self.stop_loss_trailing_price(new_stop_price);
    }

    return low <= self.stop_loss_price();
  }

  auto trigger_take_profit(this TradePosition& self, double high, double low)
   -> bool
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

  double stop_loss_initial_price_;
  double stop_loss_trailing_price_;
  double take_profit_price_;

  std::size_t entry_index_;
  std::time_t entry_timestamp_;

  std::vector<TradeRecord> realized_records_;
};

} // namespace pludux::backtest
