module;

#include <cmath>
#include <cstddef>
#include <ctime>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

export module pludux.backtest:trade_session;

import :closed_trade;
import :open_position_snapshot;
import :trade_entry;
import :trade_event;
import :trade_exit;
import :trade_position;
import :take_profit_level;
import :signal_exit_state;
import :stop_loss_level;

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
  , trade_events_{}
  , closed_trades_{}
  , realized_exits_{}
  , next_trade_id_{1}
  , next_event_id_{1}
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

  auto trade_events(this const TradeSession& self) noexcept
   -> const std::vector<TradeEvent>&
  {
    return self.trade_events_;
  }

  void strategy_index(this TradeSession& self, std::size_t value) noexcept
  {
    self.strategy_index_ = value;
  }

  auto closed_trades(this const TradeSession& self) noexcept
   -> const std::vector<ClosedTrade>&
  {
    return self.closed_trades_;
  }

  auto realized_exits(this const TradeSession& self) noexcept
   -> const std::vector<ClosedTrade>&
  {
    return self.realized_exits_;
  }

  void begin_market_bar(this TradeSession& self,
                        std::time_t timestamp,
                        double price,
                        std::size_t lookback) noexcept
  {
    self.market_timestamp_ = timestamp;
    self.market_price_ = price;
    self.market_lookback_ = lookback;
    self.trade_events_.clear();
    self.closed_trades_.clear();
    self.realized_exits_.clear();
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
      auto event = self.open_position_->scaled_in(self.next_event_id_++,
                                                  entry.position_size(),
                                                  self.market_timestamp_,
                                                  entry.price(),
                                                  total_fees);
      self.trade_events_.push_back(std::move(event));
      self.trade_events_.back().strategy_index(
       self.open_position_->strategy_index());
      return;
    }

    const auto trade_id = self.next_trade_id_++;
    self.open_position_ = TradePosition{trade_id,
                                         self.strategy_index_,
                                        entry.position_size(),
                                        self.market_timestamp_,
                                        entry.price(),
                                        total_fees};
    self.trade_events_.emplace_back(trade_id,
                                    self.next_event_id_++,
                                    1,
                                    TradeEvent::Type::entry,
                                    self.market_timestamp_,
                                    entry.price(),
                                    entry.position_size(),
                                    total_fees,
                                    0.0,
                                    0.0,
                                    0.0,
                                    self.open_position_->position_size(),
                                    self.open_position_->investment(),
                                    self.open_position_->average_price(),
                                    self.open_position_->stop_loss_levels(),
                                    self.open_position_->take_profit_levels(),
                                    self.open_position_->signal_exit_states());
     self.trade_events_.back().strategy_index(
      self.open_position_ ? self.open_position_->strategy_index()
                          : self.strategy_index_);
  }

  void reject_insufficient_cash(this TradeSession& self,
                                const TradeEntry& entry,
                                double available_cash,
                                double required_cash)
  {
    const auto trade_id =
     self.open_position_ ? self.open_position_->trade_id() : 0;
    const auto trade_event_index =
     self.open_position_ ? self.open_position_->trade_event_count() + 1 : 0;
    const auto position_size_before =
     self.open_position_ ? self.open_position_->position_size() : 0.0;
    const auto investment_before =
     self.open_position_ ? self.open_position_->investment() : 0.0;
    const auto average_price_before =
     self.open_position_ ? self.open_position_->average_price() : 0.0;

    self.trade_events_.emplace_back(
     trade_id,
     self.next_event_id_++,
     trade_event_index,
     TradeEvent::Type::rejected_insufficient_cash,
     self.market_timestamp_,
     entry.price(),
     entry.position_size(),
     0.0,
     position_size_before,
     investment_before,
     average_price_before,
     position_size_before,
     investment_before,
     average_price_before,
     self.open_position_ ? self.open_position_->stop_loss_levels()
                         : std::vector<StopLossLevel>{},
     self.open_position_ ? self.open_position_->take_profit_levels()
                         : std::vector<TakeProfitLevel>{},
     self.open_position_ ? self.open_position_->signal_exit_states()
                         : std::vector<SignalExitState>{},
     NAN,
     NAN,
     NAN,
     available_cash,
     required_cash);
     self.trade_events_.back().strategy_index(
      self.open_position_ ? self.open_position_->strategy_index()
                          : self.strategy_index_);
  }

  void reject_maximum_open_trades(this TradeSession& self,
                                  const TradeEntry& entry)
  {
    self.trade_events_.emplace_back(
     0,
     self.next_event_id_++,
     0,
     TradeEvent::Type::rejected_maximum_open_trades,
     self.market_timestamp_,
     entry.price(),
     entry.position_size(),
     0.0,
     0.0,
     0.0,
     0.0,
     0.0,
     0.0,
     0.0);
     self.trade_events_.back().strategy_index(
      self.open_position_ ? self.open_position_->strategy_index()
                          : self.strategy_index_);
  }

  void reject_maximum_combined_layers(this TradeSession& self,
                                      const TradeEntry& entry)
  {
    const auto trade_id =
     self.open_position_ ? self.open_position_->trade_id() : 0;
    const auto trade_event_index =
     self.open_position_ ? self.open_position_->trade_event_count() + 1 : 0;
    const auto position_size =
     self.open_position_ ? self.open_position_->position_size() : 0.0;
    const auto investment =
     self.open_position_ ? self.open_position_->investment() : 0.0;
    const auto average_price =
     self.open_position_ ? self.open_position_->average_price() : 0.0;

    self.trade_events_.emplace_back(
     trade_id,
     self.next_event_id_++,
     trade_event_index,
     TradeEvent::Type::rejected_maximum_combined_layers,
     self.market_timestamp_,
     entry.price(),
     entry.position_size(),
     0.0,
     position_size,
     investment,
     average_price,
     position_size,
     investment,
     average_price,
     self.open_position_ ? self.open_position_->stop_loss_levels()
                         : std::vector<StopLossLevel>{},
     self.open_position_ ? self.open_position_->take_profit_levels()
                         : std::vector<TakeProfitLevel>{},
     self.open_position_ ? self.open_position_->signal_exit_states()
                         : std::vector<SignalExitState>{});
     self.trade_events_.back().strategy_index(
      self.open_position_ ? self.open_position_->strategy_index()
                          : self.strategy_index_);
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

    const auto event_type = [](TradeExit::Reason reason) {
      switch(reason) {
      case TradeExit::Reason::stop_loss:
        return TradeEvent::Type::stop_loss;
      case TradeExit::Reason::take_profit:
        return TradeEvent::Type::take_profit;
      case TradeExit::Reason::signal:
      default:
        return TradeEvent::Type::exit_signal;
      }
    }(exit.reason());

    const auto closed_position_size = exit.position_size();
    const auto closed_investment =
     closed_position_size * self.open_position_->average_price();
    if(exit.reason() == TradeExit::Reason::take_profit &&
       (!exit.take_profit_index() ||
        *exit.take_profit_index() >=
         self.open_position_->take_profit_levels().size())) {
      throw std::runtime_error{
       "Take-profit exit requires a valid target index."};
    }
    if(exit.reason() == TradeExit::Reason::stop_loss &&
       (!exit.stop_loss_index() ||
        *exit.stop_loss_index() >=
         self.open_position_->stop_loss_levels().size())) {
      throw std::runtime_error{"Stop-loss exit requires a valid stop index."};
    }
    if(exit.signal_exit_index() &&
       *exit.signal_exit_index() >=
        self.open_position_->signal_exit_states().size()) {
      throw std::runtime_error{"Signal exit requires a valid exit index."};
    }
    const auto exit_event_id = self.next_event_id_++;
    const auto owner_strategy_index = self.open_position_->strategy_index();
    auto event = self.open_position_->scaled_out(exit_event_id,
                                                 exit.position_size(),
                                                 self.market_timestamp_,
                                                 exit.price(),
                                                 total_fees,
                                                 event_type);
    if(const auto take_profit_index = exit.take_profit_index()) {
      self.open_position_->take_profit_levels()[*take_profit_index].consumed(
       true);
    }
    if(const auto stop_loss_index = exit.stop_loss_index()) {
      self.open_position_->stop_loss_levels()[*stop_loss_index].consumed(true);
    }
    if(const auto signal_exit_index = exit.signal_exit_index()) {
      self.open_position_->signal_exit_states()[*signal_exit_index].consumed(
       true);
    }
    event.after_state(self.open_position_->position_size(),
                      self.open_position_->investment(),
                      self.open_position_->average_price(),
                      self.open_position_->stop_loss_levels(),
                      self.open_position_->take_profit_levels(),
                      self.open_position_->signal_exit_states(),
                      self.open_position_->risk_distance(),
                      self.open_position_->risk_reference_price(),
                      self.open_position_->risk_boundary_price());
    const auto closed_trade =
     self.open_position_->closed_trade(exit_event_id,
                                       event_type,
                                       self.market_timestamp_,
                                       exit.price(),
                                       total_fees,
                                       closed_position_size,
                                       closed_investment);
    self.realized_exits_.push_back(closed_trade);
    self.trade_events_.push_back(std::move(event));
    self.trade_events_.back().strategy_index(owner_strategy_index);

    if(self.open_position_->is_closed()) {
      self.closed_trades_.push_back(closed_trade);
      self.open_position_ = std::nullopt;
    }
  }

  auto open_position_snapshot(this const TradeSession& self) noexcept
   -> std::optional<OpenPositionSnapshot>
  {
    if(!self.open_position_) {
      return std::nullopt;
    }

    return self.open_position_->snapshot(self.market_timestamp_,
                                         self.market_price_);
  }

  void sync_latest_event_with_open_position(this TradeSession& self) noexcept
  {
    if(!self.open_position_ || self.trade_events_.empty()) {
      return;
    }

    auto& event = self.trade_events_.back();
    const auto& position = *self.open_position_;
    event.after_state(position.position_size(),
                      position.investment(),
                      position.average_price(),
                      position.stop_loss_levels(),
                      position.take_profit_levels(),
                      position.signal_exit_states(),
                      position.risk_distance(),
                      position.risk_reference_price(),
                      position.risk_boundary_price());
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
  std::vector<TradeEvent> trade_events_;
  std::vector<ClosedTrade> closed_trades_;
  std::vector<ClosedTrade> realized_exits_;
  std::size_t next_trade_id_;
  std::size_t next_event_id_;
  std::size_t strategy_index_{};
};

} // namespace pludux::backtest
