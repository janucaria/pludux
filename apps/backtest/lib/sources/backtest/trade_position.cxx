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
                std::size_t entry_index,
                double stop_loss_initial_price,
                double stop_loss_trailing_price,
                double take_profit_price)
  : TradePosition{position_size,
                  entry_price,
                  entry_timestamp,
                  entry_price,
                  entry_index,
                  stop_loss_initial_price,
                  stop_loss_trailing_price,
                  take_profit_price,
                  {}}
  {
  }

  TradePosition(double position_size,
                double average_price,
                std::time_t entry_timestamp,
                double entry_price,
                std::size_t entry_index,
                double stop_loss_initial_price,
                double stop_loss_trailing_price,
                double take_profit_price,
                std::vector<TradeRecord> realized_records)
  : position_size_{position_size}
  , average_price_{average_price}
  , entry_price_{entry_price}
  , stop_loss_initial_price_{stop_loss_initial_price}
  , stop_loss_trailing_price_{stop_loss_trailing_price}
  , take_profit_price_{take_profit_price}
  , entry_timestamp_{entry_timestamp}
  , entry_index_{entry_index}
  , realized_records_{std::move(realized_records)}
  {
  }

  auto position_size(this const auto& self) noexcept -> double
  {
    return self.position_size_;
  }

  void position_size(this auto& self, double size) noexcept
  {
    self.position_size_ = size;
  }

  auto average_price(this const auto& self) noexcept -> double
  {
    return self.average_price_;
  }

  void average_price(this auto& self, double price) noexcept
  {
    self.average_price_ = price;
  }

  auto entry_price(this const auto& self) noexcept -> double
  {
    return self.entry_price_;
  }

  void entry_price(this auto& self, double price) noexcept
  {
    self.entry_price_ = price;
  }

  auto stop_loss_initial_price(this const auto& self) noexcept -> double
  {
    return self.stop_loss_initial_price_;
  }

  void stop_loss_initial_price(this auto& self, double price) noexcept
  {
    self.stop_loss_initial_price_ = price;
  }

  auto stop_loss_trailing_price(this const auto& self) noexcept -> double
  {
    return self.stop_loss_trailing_price_;
  }

  void stop_loss_trailing_price(this auto& self, double price) noexcept
  {
    self.stop_loss_trailing_price_ = price;
  }

  auto take_profit_price(this const auto& self) noexcept -> double
  {
    return self.take_profit_price_;
  }

  void take_profit_price(this auto& self, double price) noexcept
  {
    self.take_profit_price_ = price;
  }

  auto entry_index(this const auto& self) noexcept -> std::size_t
  {
    return self.entry_index_;
  }

  void entry_index(this auto& self, std::size_t index) noexcept
  {
    self.entry_index_ = index;
  }

  auto entry_timestamp(this const auto& self) noexcept -> std::time_t
  {
    return self.entry_timestamp_;
  }

  void entry_timestamp(this auto& self, std::time_t timestamp) noexcept
  {
    self.entry_timestamp_ = timestamp;
  }

  auto realized_records(this const auto& self) noexcept
   -> const std::vector<TradeRecord>&
  {
    return self.realized_records_;
  }

  void realized_records(this auto& self,
                        std::vector<TradeRecord> records) noexcept
  {
    self.realized_records_ = std::move(records);
  }

  auto stop_loss_price(this const auto& self) noexcept -> double
  {
    return std::isnan(self.stop_loss_trailing_price())
            ? self.stop_loss_initial_price()
            : self.stop_loss_trailing_price();
  }

  auto unrealized_position_size(this const auto& self) noexcept -> double
  {
    return self.position_size();
  }

  auto unrealized_investment(this const auto& self) noexcept -> double
  {
    return self.unrealized_position_size() * self.average_price();
  }

  auto unrealized_pnl(this const auto& self, double market_price) noexcept
   -> double
  {
    return self.unrealized_position_size() *
           (market_price - self.average_price());
  }

  auto unrealized_duration(this const auto& self,
                           std::time_t market_timestamp) noexcept -> std::time_t
  {
    return market_timestamp - self.entry_timestamp();
  }

  auto realized_position_size(this const auto& self) noexcept -> double
  {
    return std::ranges::fold_left(
     self.realized_records(), 0.0, [](double total, const TradeRecord& record) {
       return total + record.position_size();
     });
  }

  auto realized_investment(this const auto& self) noexcept -> double
  {
    return std::ranges::fold_left(
     self.realized_records(), 0.0, [](double total, const TradeRecord& record) {
       return total + record.investment();
     });
  }

  auto realized_pnl(this const auto& self) noexcept -> double
  {
    return std::ranges::fold_left(
     self.realized_records(), 0.0, [](double total, const TradeRecord& record) {
       return total + record.pnl();
     });
  }

  auto realized_duration(this const auto& self) noexcept -> std::time_t
  {
    return std::ranges::fold_left(
     self.realized_records(),
     std::time_t{},
     [](std::time_t total, const TradeRecord& record) {
       return total + record.duration();
     });
  }

  auto is_closed(this const auto& self) noexcept -> bool
  {
    return self.position_size() == 0;
  }

  auto is_long_direction(this const auto& self) noexcept -> bool
  {
    return self.position_size() > 0;
  }

  auto is_short_direction(this const auto& self) noexcept -> bool
  {
    return self.position_size() < 0;
  }

  void scaled_in(this auto& self,
                 double action_position_size,
                 std::time_t action_timestamp,
                 double action_price,
                 std::size_t action_index)
  {
    if(self.is_closed()) {
      throw std::runtime_error("Cannot scaled in to a closed trade.");
    }

    const auto last_position_size = self.position_size();
    const auto last_average_price = self.average_price();
    const auto last_investment = last_position_size * last_average_price;

    const auto new_position_size = last_position_size + action_position_size;
    const auto new_average_price =
     (last_investment + action_position_size * action_price) /
     new_position_size;

    auto scaled_in_record = TradeRecord{TradeRecord::Status::scaled_in,
                                        0.0,
                                        last_average_price,
                                        self.entry_timestamp(),
                                        self.entry_price(),
                                        self.entry_index(),
                                        action_timestamp,
                                        action_price,
                                        action_index,
                                        self.stop_loss_initial_price(),
                                        self.stop_loss_trailing_price(),
                                        self.take_profit_price()};

    self.position_size(new_position_size);
    self.average_price(new_average_price);

    self.realized_records_.emplace_back(std::move(scaled_in_record));
  }

  void scaled_out(this auto& self,
                  double action_position_size,
                  std::time_t action_timestamp,
                  double action_price,
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

    auto exit_record = TradeRecord{trade_status,
                                   action_position_size,
                                   self.average_price(),
                                   self.entry_timestamp(),
                                   self.entry_price(),
                                   self.entry_index(),
                                   action_timestamp,
                                   action_price,
                                   action_index,
                                   self.stop_loss_initial_price(),
                                   self.stop_loss_trailing_price(),
                                   self.take_profit_price()};
    self.realized_records_.emplace_back(std::move(exit_record));

    self.position_size(remaining_position_size);
  }

  auto
  trigger_stop_loss(this auto& self, double prev_close, double high, double low)
   -> bool
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

  auto trigger_take_profit(this auto& self, double high, double low) -> bool
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
