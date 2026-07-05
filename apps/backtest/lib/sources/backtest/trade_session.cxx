module;

#include <cmath>
#include <cstddef>
#include <ctime>
#include <optional>
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
  TradeSession()
  : TradeSession{0, NAN, 0}
  {
  }

  TradeSession(std::time_t market_timestamp,
               double market_price,
               std::size_t market_lookback)
  : TradeSession{market_timestamp, market_price, market_lookback, std::nullopt}
  {
  }

  TradeSession(std::time_t market_timestamp,
               double market_price,
               std::size_t market_lookback,
               std::optional<TradePosition> open_position)
  : market_timestamp_{market_timestamp}
  , market_price_{market_price}
  , market_lookback_{market_lookback}
  , open_position_{std::move(open_position)}
  , trade_records_{}
  {
  }

  auto operator==(const TradeSession&) const noexcept -> bool = default;

  auto market_timestamp(this const TradeSession& self) noexcept -> std::time_t
  {
    return self.market_timestamp_;
  }

  auto market_price(this const TradeSession& self) noexcept -> double
  {
    return self.market_price_;
  }

  auto market_lookback(this const TradeSession& self) noexcept -> std::size_t
  {
    return self.market_lookback_;
  }

  auto open_position(this const TradeSession& self) noexcept
   -> const std::optional<TradePosition>&
  {
    return self.open_position_;
  }

  auto open_position(this TradeSession& self) noexcept
   -> std::optional<TradePosition>&
  {
    return self.open_position_;
  }

  auto trade_records(this const TradeSession& self) noexcept
   -> const std::vector<TradeRecord>&
  {
    return self.trade_records_;
  }

  void begin_market_bar(this TradeSession& self,
                        std::time_t timestamp,
                        double price,
                        std::size_t lookback) noexcept
  {
    self.market_timestamp_ = timestamp;
    self.market_price_ = price;
    self.market_lookback_ = lookback;
    self.trade_records_.clear();
  }

  void entry_position(this TradeSession& self, const TradeEntry& entry)
  {
    self.entry_position(entry, 0.0);
  }

  void entry_position(this TradeSession& self,
                      const TradeEntry& entry,
                      double total_fees)
  {
    if(self.open_position_) {
      auto trade_record =
       self.open_position_->scaled_in(entry.position_size(),
                                      self.market_timestamp_,
                                      entry.price(),
                                      total_fees,
                                      entry.stop_price(),
                                      entry.target_price(),
                                      entry.stop_loss_price(),
                                      entry.stop_loss_trailing_enabled(),
                                      entry.take_profit_price());
      self.trade_records_.push_back(std::move(trade_record));
      return;
    }

    self.open_position_ = TradePosition{entry.position_size(),
                                        self.market_timestamp_,
                                        entry.price(),
                                        total_fees,
                                        entry.stop_price(),
                                        entry.target_price(),
                                        entry.stop_loss_price(),
                                        entry.stop_loss_trailing_enabled(),
                                        entry.take_profit_price()};
  }

  void exit_position(this TradeSession& self, const TradeExit& exit)
  {
    self.exit_position(exit, 0.0);
  }

  void exit_position(this TradeSession& self,
                     const TradeExit& exit,
                     double total_fees)
  {
    if(!self.open_position_) {
      throw std::runtime_error{"Cannot exit a closed trade."};
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

    auto trade_record = self.open_position_->scaled_out(exit.position_size(),
                                                        self.market_timestamp_,
                                                        exit.price(),
                                                        total_fees,
                                                        trade_status);
    self.trade_records_.push_back(std::move(trade_record));

    if(self.open_position_->is_closed()) {
      self.open_position_ = std::nullopt;
    }
  }

  auto unrealized_pnl(this const TradeSession& self) noexcept -> double
  {
    return self.open_position_
            ? self.open_position_->unrealized_pnl(self.market_price_)
            : 0.0;
  }

  auto unrealized_investment(this const TradeSession& self) noexcept -> double
  {
    return self.open_position_ ? self.open_position_->unrealized_investment()
                               : 0.0;
  }

  auto unrealized_duration(this const TradeSession& self) noexcept
   -> std::time_t
  {
    return self.open_position_
            ? self.open_position_->unrealized_duration(self.market_timestamp_)
            : 0;
  }

  auto is_flat(this const TradeSession& self) noexcept -> bool
  {
    return !self.is_open();
  }

  auto is_open(this const TradeSession& self) noexcept -> bool
  {
    return self.open_position_.has_value();
  }

private:
  std::time_t market_timestamp_;
  double market_price_;
  std::size_t market_lookback_;

  std::optional<TradePosition> open_position_;
  std::vector<TradeRecord> trade_records_;
};

} // namespace pludux::backtest
