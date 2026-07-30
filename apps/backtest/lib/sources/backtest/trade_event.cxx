module;

#include <cmath>
#include <cstddef>
#include <ctime>
#include <utility>
#include <vector>

export module pludux.backtest:trade_event;

import :take_profit_level;
import :signal_exit_state;
import :stop_loss_level;

export namespace pludux::backtest {

class TradeEvent {
public:
  enum class Type {
    entry,
    scale_in,
    scale_out,
    exit_signal,
    stop_loss,
    take_profit,
    rejected_insufficient_cash
  };

  TradeEvent() = default;

  TradeEvent(std::size_t trade_id,
             std::size_t event_id,
             std::size_t trade_event_index,
             Type type,
             std::time_t timestamp,
             double price,
             double position_size,
             double fees,
             double position_size_before,
             double investment_before,
             double average_price_before,
             double position_size_after,
             double investment_after,
             double average_price_after,
             std::vector<StopLossLevel> stop_loss_levels = {},
             std::vector<TakeProfitLevel> take_profit_levels = {},
             std::vector<SignalExitState> signal_exit_states = {},
             double risk_distance = NAN,
             double risk_reference_price = NAN,
             double risk_boundary_price = NAN)
  : trade_id_{trade_id}
  , event_id_{event_id}
  , trade_event_index_{trade_event_index}
  , type_{type}
  , timestamp_{timestamp}
  , price_{price}
  , position_size_{position_size}
  , fees_{fees}
  , position_size_before_{position_size_before}
  , investment_before_{investment_before}
  , average_price_before_{average_price_before}
  , position_size_after_{position_size_after}
  , investment_after_{investment_after}
  , average_price_after_{average_price_after}
  , stop_loss_levels_{std::move(stop_loss_levels)}
  , take_profit_levels_{std::move(take_profit_levels)}
  , signal_exit_states_{std::move(signal_exit_states)}
  , risk_distance_{risk_distance}
  , risk_reference_price_{risk_reference_price}
  , risk_boundary_price_{risk_boundary_price}
  {
  }

  auto operator==(const TradeEvent&) const noexcept -> bool = default;

  auto trade_id(this const TradeEvent& self) noexcept -> std::size_t
  {
    return self.trade_id_;
  }

  auto event_id(this const TradeEvent& self) noexcept -> std::size_t
  {
    return self.event_id_;
  }

  auto trade_event_index(this const TradeEvent& self) noexcept -> std::size_t
  {
    return self.trade_event_index_;
  }

  auto type(this const TradeEvent& self) noexcept -> Type
  {
    return self.type_;
  }

  auto timestamp(this const TradeEvent& self) noexcept -> std::time_t
  {
    return self.timestamp_;
  }

  auto price(this const TradeEvent& self) noexcept -> double
  {
    return self.price_;
  }

  auto position_size(this const TradeEvent& self) noexcept -> double
  {
    return self.position_size_;
  }

  auto fees(this const TradeEvent& self) noexcept -> double
  {
    return self.fees_;
  }

  auto position_size_before(this const TradeEvent& self) noexcept -> double
  {
    return self.position_size_before_;
  }

  auto investment_before(this const TradeEvent& self) noexcept -> double
  {
    return self.investment_before_;
  }

  auto average_price_before(this const TradeEvent& self) noexcept -> double
  {
    return self.average_price_before_;
  }

  auto position_size_after(this const TradeEvent& self) noexcept -> double
  {
    return self.position_size_after_;
  }

  auto investment_after(this const TradeEvent& self) noexcept -> double
  {
    return self.investment_after_;
  }

  auto average_price_after(this const TradeEvent& self) noexcept -> double
  {
    return self.average_price_after_;
  }

  auto stop_loss_levels(this const TradeEvent& self) noexcept
   -> const std::vector<StopLossLevel>&
  {
    return self.stop_loss_levels_;
  }

  auto take_profit_levels(this const TradeEvent& self) noexcept
   -> const std::vector<TakeProfitLevel>&
  {
    return self.take_profit_levels_;
  }

  auto signal_exit_states(this const TradeEvent& self) noexcept
   -> const std::vector<SignalExitState>&
  {
    return self.signal_exit_states_;
  }

  auto risk_distance(this const TradeEvent& self) noexcept -> double
  {
    return self.risk_distance_;
  }

  auto risk_reference_price(this const TradeEvent& self) noexcept -> double
  {
    return self.risk_reference_price_;
  }

  auto risk_boundary_price(this const TradeEvent& self) noexcept -> double
  {
    return self.risk_boundary_price_;
  }

  void after_state(this TradeEvent& self,
                   double position_size,
                   double investment,
                   double average_price,
                   std::vector<StopLossLevel> stop_loss_levels = {},
                   std::vector<TakeProfitLevel> take_profit_levels = {},
                   std::vector<SignalExitState> signal_exit_states = {},
                   double risk_distance = NAN,
                   double risk_reference_price = NAN,
                   double risk_boundary_price = NAN) noexcept
  {
    self.position_size_after_ = position_size;
    self.investment_after_ = investment;
    self.average_price_after_ = average_price;
    self.stop_loss_levels_ = std::move(stop_loss_levels);
    self.take_profit_levels_ = std::move(take_profit_levels);
    self.signal_exit_states_ = std::move(signal_exit_states);
    self.risk_distance_ = risk_distance;
    self.risk_reference_price_ = risk_reference_price;
    self.risk_boundary_price_ = risk_boundary_price;
  }

  auto is_entry(this const TradeEvent& self) noexcept -> bool
  {
    return self.type_ == Type::entry;
  }

  auto is_scale_in(this const TradeEvent& self) noexcept -> bool
  {
    return self.type_ == Type::scale_in;
  }

  auto is_scale_out(this const TradeEvent& self) noexcept -> bool
  {
    const auto is_reason_specific_exit = self.type_ == Type::exit_signal ||
                                         self.type_ == Type::stop_loss ||
                                         self.type_ == Type::take_profit;

    return self.type_ == Type::scale_out ||
           (is_reason_specific_exit && self.position_size_after_ != 0.0);
  }

  auto is_exit(this const TradeEvent& self) noexcept -> bool
  {
    return self.type_ == Type::exit_signal || self.type_ == Type::stop_loss ||
           self.type_ == Type::take_profit;
  }

  auto is_rejected(this const TradeEvent& self) noexcept -> bool
  {
    return self.type_ == Type::rejected_insufficient_cash;
  }

private:
  std::size_t trade_id_{};
  std::size_t event_id_{};
  std::size_t trade_event_index_{};
  Type type_{Type::entry};

  std::time_t timestamp_{};
  double price_{};
  double position_size_{};
  double fees_{};

  double position_size_before_{};
  double investment_before_{};
  double average_price_before_{};

  double position_size_after_{};
  double investment_after_{};
  double average_price_after_{};

  std::vector<StopLossLevel> stop_loss_levels_;
  std::vector<TakeProfitLevel> take_profit_levels_;
  std::vector<SignalExitState> signal_exit_states_;
  double risk_distance_{NAN};
  double risk_reference_price_{NAN};
  double risk_boundary_price_{NAN};
};

} // namespace pludux::backtest
