module;

#include <cmath>
#include <cstddef>
#include <ctime>
#include <utility>
#include <vector>

export module pludux.backtest:closed_trade;

import :trade_event;
import :take_profit_level;
import :signal_exit_state;
import :stop_loss_level;

export namespace pludux::backtest {

class ClosedTrade {
public:
  ClosedTrade() = default;

  ClosedTrade(std::size_t trade_id,
              std::size_t setup_index,
              std::size_t exit_event_id,
              TradeEvent::Type exit_type,
              std::time_t entry_timestamp,
              std::time_t exit_timestamp,
              double position_size,
              double investment,
              double entry_price,
              double exit_price,
              double total_entry_fees,
              double total_exit_fees,
              std::vector<StopLossLevel> stop_loss_levels = {},
              std::vector<TakeProfitLevel> take_profit_levels = {},
              std::vector<SignalExitState> signal_exit_states = {},
              double risk_distance = NAN,
              double risk_reference_price = NAN,
              double risk_boundary_price = NAN)
  : trade_id_{trade_id}
  , setup_index_{setup_index}
  , exit_event_id_{exit_event_id}
  , exit_type_{exit_type}
  , entry_timestamp_{entry_timestamp}
  , exit_timestamp_{exit_timestamp}
  , position_size_{position_size}
  , investment_{investment}
  , entry_price_{entry_price}
  , exit_price_{exit_price}
  , total_entry_fees_{total_entry_fees}
  , total_exit_fees_{total_exit_fees}
  , stop_loss_levels_{std::move(stop_loss_levels)}
  , take_profit_levels_{std::move(take_profit_levels)}
  , signal_exit_states_{std::move(signal_exit_states)}
  , risk_distance_{risk_distance}
  , risk_reference_price_{risk_reference_price}
  , risk_boundary_price_{risk_boundary_price}
  {
  }

  auto operator==(const ClosedTrade&) const noexcept -> bool = default;

  auto trade_id(this const ClosedTrade& self) noexcept -> std::size_t
  {
    return self.trade_id_;
  }

  auto setup_index(this const ClosedTrade& self) noexcept -> std::size_t
  {
    return self.setup_index_;
  }

  auto exit_event_id(this const ClosedTrade& self) noexcept -> std::size_t
  {
    return self.exit_event_id_;
  }

  auto exit_type(this const ClosedTrade& self) noexcept -> TradeEvent::Type
  {
    return self.exit_type_;
  }

  auto entry_timestamp(this const ClosedTrade& self) noexcept -> std::time_t
  {
    return self.entry_timestamp_;
  }

  auto exit_timestamp(this const ClosedTrade& self) noexcept -> std::time_t
  {
    return self.exit_timestamp_;
  }

  auto position_size(this const ClosedTrade& self) noexcept -> double
  {
    return self.position_size_;
  }

  auto investment(this const ClosedTrade& self) noexcept -> double
  {
    return self.investment_;
  }

  auto entry_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.entry_price_;
  }

  auto exit_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.exit_price_;
  }

  auto total_entry_fees(this const ClosedTrade& self) noexcept -> double
  {
    return self.total_entry_fees_;
  }

  auto total_exit_fees(this const ClosedTrade& self) noexcept -> double
  {
    return self.total_exit_fees_;
  }

  auto stop_loss_levels(this const ClosedTrade& self) noexcept
   -> const std::vector<StopLossLevel>&
  {
    return self.stop_loss_levels_;
  }

  auto take_profit_levels(this const ClosedTrade& self) noexcept
   -> const std::vector<TakeProfitLevel>&
  {
    return self.take_profit_levels_;
  }

  auto signal_exit_states(this const ClosedTrade& self) noexcept
   -> const std::vector<SignalExitState>&
  {
    return self.signal_exit_states_;
  }

  auto risk_distance(this const ClosedTrade& self) noexcept -> double
  {
    return self.risk_distance_;
  }

  auto risk_reference_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.risk_reference_price_;
  }

  auto risk_boundary_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.risk_boundary_price_;
  }

  auto average_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.position_size_ ? self.investment_ / self.position_size_ : 0.0;
  }

  auto exit_value(this const ClosedTrade& self) noexcept -> double
  {
    return self.position_size_ * self.exit_price_;
  }

  auto pnl(this const ClosedTrade& self) noexcept -> double
  {
    return self.position_size_
            ? self.exit_value() - self.total_exit_fees_ - self.investment_
            : 0.0;
  }

  auto duration(this const ClosedTrade& self) noexcept -> std::time_t
  {
    return self.exit_timestamp_ - self.entry_timestamp_;
  }

private:
  std::size_t trade_id_{};
  std::size_t setup_index_{};
  std::size_t exit_event_id_{};
  TradeEvent::Type exit_type_{TradeEvent::Type::exit_signal};

  std::time_t entry_timestamp_{};
  std::time_t exit_timestamp_{};
  double position_size_{};
  double investment_{};
  double entry_price_{};
  double exit_price_{};
  double total_entry_fees_{};
  double total_exit_fees_{};
  std::vector<StopLossLevel> stop_loss_levels_;
  std::vector<TakeProfitLevel> take_profit_levels_;
  std::vector<SignalExitState> signal_exit_states_;
  double risk_distance_{NAN};
  double risk_reference_price_{NAN};
  double risk_boundary_price_{NAN};
};

} // namespace pludux::backtest
