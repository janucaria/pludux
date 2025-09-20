module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <functional>
#include <iterator>
#include <limits>
#include <numeric>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

export module pludux.backtest:trade_session;

import :trade_entry;
import :trade_exit;
import :trade_position;
import :trade_record;

export namespace pludux::backtest {

class TradeSession {
public:
  class ConstTradeRecordIterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = TradeRecord;
    using difference_type = std::ptrdiff_t;

    ConstTradeRecordIterator(const TradeSession& session)
    : session_{session}
    , closed_position_index_{0}
    , open_position_index_{0}
    , waiting_for_open_record_{session.open_position().has_value()}
    {
    }

    auto operator*(this const auto& self) -> TradeRecord
    {
      const auto& closed_position = self.session_.closed_position();
      if(closed_position && self.closed_position_index_ <
                             closed_position->realized_records().size()) {
        return closed_position->realized_records().at(
         self.closed_position_index_);
      }

      const auto& open_position = self.session_.open_position();
      if(self.open_position_index_ < open_position->realized_records().size()) {
        return open_position->realized_records().at(self.open_position_index_);
      }

      return TradeRecord{TradeRecord::Status::open,
                         open_position->position_size(),
                         open_position->average_price(),
                         open_position->entry_timestamp(),
                         open_position->entry_price(),
                         open_position->entry_index(),
                         self.session_.market_timestamp(),
                         self.session_.market_price(),
                         self.session_.market_index(),
                         open_position->stop_loss_initial_price(),
                         open_position->stop_loss_trailing_price(),
                         open_position->take_profit_price()};
    }

    auto operator++(this auto& self) -> ConstTradeRecordIterator&
    {
      const auto& closed_position = self.session_.closed_position();
      if(closed_position && self.closed_position_index_ <
                             closed_position->realized_records().size()) {
        self.closed_position_index_++;
      } else {
        const auto& open_position = self.session_.open_position();
        if(open_position && self.open_position_index_ <
                             open_position->realized_records().size()) {
          self.open_position_index_++;
        } else if(self.waiting_for_open_record_) {
          self.waiting_for_open_record_ = false;
        }
      }

      return self;
    }

    auto operator++(this auto& self, int) -> ConstTradeRecordIterator
    {
      auto tmp = self;
      ++self;
      return tmp;
    }

    auto operator==(this const auto& self, std::default_sentinel_t) -> bool
    {
      const auto& closed_position = self.session_.closed_position();
      const auto& open_position = self.session_.open_position();

      const auto closed_position_size =
       closed_position ? closed_position->realized_records().size() : 0;
      const auto open_position_size =
       open_position ? open_position->realized_records().size() : 0;

      return self.closed_position_index_ >= closed_position_size &&
             self.open_position_index_ >= open_position_size &&
             !self.waiting_for_open_record_;
    }

  private:
    const TradeSession& session_;
    std::size_t closed_position_index_;
    std::size_t open_position_index_;
    bool waiting_for_open_record_;
  };

  class ConstTradeRecordRange {
  public:
    ConstTradeRecordRange(const TradeSession& session)
    : session_{session}
    {
    }

    auto begin(this const auto& self) -> ConstTradeRecordIterator
    {
      return ConstTradeRecordIterator{self.session_};
    }

    auto end(this const auto&) -> std::default_sentinel_t
    {
      return std::default_sentinel;
    }

  private:
    const TradeSession& session_;
  };

  TradeSession()
  : TradeSession{0, NAN, 0}
  {
  }

  TradeSession(std::time_t market_timestamp_,
               double market_price_,
               std::size_t market_index_)
  : TradeSession{market_timestamp_,
                 market_price_,
                 market_index_,
                 std::nullopt,
                 std::nullopt}
  {
  }

  TradeSession(std::time_t market_timestamp_,
               double market_price_,
               std::size_t market_index_,
               std::optional<TradePosition> open_position,
               std::optional<TradePosition> closed_position)
  : market_timestamp_{market_timestamp_}
  , market_price_{market_price_}
  , market_index_{market_index_}
  , open_position_{std::move(open_position)}
  , closed_position_{std::move(closed_position)}
  {
  }

  auto market_timestamp(this const auto& self) noexcept -> std::time_t
  {
    return self.market_timestamp_;
  }

  void market_timestamp(this auto& self, std::time_t timestamp) noexcept
  {
    self.market_timestamp_ = timestamp;
  }

  auto market_price(this const auto& self) noexcept -> double
  {
    return self.market_price_;
  }

  void market_price(this auto& self, double price) noexcept
  {
    self.market_price_ = price;
  }

  auto market_index(this const auto& self) noexcept -> std::size_t
  {
    return self.market_index_;
  }

  void market_index(this auto& self, std::size_t index) noexcept
  {
    self.market_index_ = index;
  }

  auto open_position(this const auto& self) noexcept
   -> const std::optional<TradePosition>&
  {
    return self.open_position_;
  }

  void open_position(this auto& self,
                     std::optional<TradePosition> position) noexcept
  {
    self.open_position_ = std::move(position);
  }

  auto closed_position(this const auto& self) noexcept
   -> const std::optional<TradePosition>&
  {
    return self.closed_position_;
  }

  void closed_position(this auto& self,
                       std::optional<TradePosition> position) noexcept
  {
    self.closed_position_ = std::move(position);
  }

  void entry_position(this auto& self, const TradeEntry& entry) noexcept
  {
    if(self.open_position_) {
      self.open_position_->scaled_in(entry.position_size(),
                                     self.market_timestamp_,
                                     entry.price(),
                                     self.market_index_);

      return;
    }

    self.open_position_ = TradePosition{entry.position_size(),
                                        self.market_timestamp_,
                                        entry.price(),
                                        self.market_index_,
                                        entry.stop_loss_price(),
                                        entry.stop_loss_trailing_price(),
                                        entry.take_profit_price()};
  }

  void exit_position(this auto& self, const TradeExit& exit)
  {
    if(!self.open_position_) {
      throw std::runtime_error("Cannot exit a closed trade.");
    }

    const auto trade_status = [](TradeExit::Reason reason) {
      switch(reason) {
      case TradeExit::Reason::stop_loss:
        return TradeRecord::Status::closed_stop_loss;
      case TradeExit::Reason::take_profit:
        return TradeRecord::Status::closed_take_profit;
      case TradeExit::Reason::signal:
      default:
        return TradeRecord::Status::closed_exit_signal;
      }
    }(exit.reason());

    self.open_position_->scaled_out(exit.position_size(),
                                    self.market_timestamp_,
                                    exit.price(),
                                    self.market_index_,
                                    trade_status);

    if(self.open_position_->is_closed()) {
      self.closed_position_ = std::move(*self.open_position_);
      self.open_position_ = std::nullopt;
    }
  }

  void market_update(this auto& self,
                     std::time_t timestamp,
                     double price,
                     std::size_t index) noexcept
  {
    self.market_timestamp(timestamp);
    self.market_price(price);
    self.market_index(index);
  }

  auto trade_record_range(this const auto& self) noexcept
   -> ConstTradeRecordRange
  {
    return ConstTradeRecordRange{self};
  }

  void evaluate_exit_conditions(this auto& self,
                                double prev_close,
                                double open,
                                double high,
                                double low) noexcept
  {
    if(self.is_flat()) {
      return;
    }

    if(self.closed_position_) {
      self.closed_position_ = std::nullopt;
    }

    if(!self.open_position_) {
      return;
    }

    const auto position_size = self.open_position_->unrealized_position_size();

    if(self.open_position_->trigger_stop_loss(prev_close, high, low)) {
      const auto exit_price =
       self.open_position_->is_long_direction()
        ? std::min(open, self.open_position_->stop_loss_price())
        : std::max(open, self.open_position_->stop_loss_price());
      auto trade_exit =
       TradeExit{position_size, exit_price, TradeExit::Reason::stop_loss};

      self.exit_position(trade_exit);
    } else if(self.open_position_->trigger_take_profit(high, low)) {
      const auto exit_price =
       self.open_position_->is_long_direction()
        ? std::max(open, self.open_position_->take_profit_price())
        : std::min(open, self.open_position_->take_profit_price());
      auto trade_exit =
       TradeExit{position_size, exit_price, TradeExit::Reason::take_profit};

      self.exit_position(trade_exit);
    }
  }

  auto unrealized_pnl(this const auto& self) noexcept -> double
  {
    return self.open_position_
            ? self.open_position_->unrealized_pnl(self.market_price_)
            : 0.0;
  }

  auto partial_realized_pnl(this const auto& self) noexcept -> double
  {
    return self.open_position_ ? self.open_position_->realized_pnl() : 0.0;
  }

  auto unrealized_investment(this const auto& self) noexcept -> double
  {
    return self.open_position_ ? self.open_position_->unrealized_investment()
                               : 0.0;
  }

  auto unrealized_duration(this const auto& self) noexcept -> std::time_t
  {
    return self.open_position_
            ? self.open_position_->unrealized_duration(self.market_timestamp_)
            : 0;
  }

  auto realized_pnl(this const auto& self) noexcept -> double
  {
    return self.closed_position_ ? self.closed_position_->realized_pnl() : 0.0;
  }

  auto realized_investment(this const auto& self) noexcept -> double
  {
    return self.closed_position_ ? self.closed_position_->realized_investment()
                                 : 0.0;
  }

  auto realized_duration(this const auto& self) noexcept -> std::time_t
  {
    return self.closed_position_ ? self.closed_position_->realized_duration()
                                 : 0;
  }

  auto is_flat(this const auto& self) noexcept -> bool
  {
    return !self.open_position_ && !self.closed_position_;
  }

  auto is_open(this const auto& self) noexcept -> bool
  {
    return self.open_position_.has_value();
  }

  auto is_closed(this const auto& self) noexcept -> bool
  {
    return !self.is_open() && self.closed_position_.has_value();
  }

  auto is_reopen(this const auto& self) noexcept -> bool
  {
    return self.open_position_.has_value() && self.closed_position_.has_value();
  }

private:
  std::time_t market_timestamp_;
  double market_price_;
  std::size_t market_index_;

  std::optional<TradePosition> open_position_;

  std::optional<TradePosition> closed_position_;
};

} // namespace pludux::backtest
