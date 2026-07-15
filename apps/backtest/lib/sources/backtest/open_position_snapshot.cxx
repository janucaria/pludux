module;

#include <cmath>
#include <cstddef>
#include <ctime>
#include <utility>
#include <vector>

export module pludux.backtest:open_position_snapshot;

import :take_profit_level;
import :signal_exit_state;

export namespace pludux::backtest {

class OpenPositionSnapshot {
public:
  OpenPositionSnapshot() = default;

  OpenPositionSnapshot(std::size_t trade_id,
                       std::time_t entry_timestamp,
                       std::time_t market_timestamp,
                       double market_price,
                       double position_size,
                       double investment,
                       double entry_price,
                       double total_entry_fees,
                       double stop_price,
                       double stop_loss_price,
                       std::vector<TakeProfitLevel> take_profit_levels = {},
                       std::vector<SignalExitState> signal_exit_states = {},
                       double risk_distance = NAN,
                       double risk_reference_price = NAN,
                       double risk_boundary_price = NAN)
  : trade_id_{trade_id}
  , entry_timestamp_{entry_timestamp}
  , market_timestamp_{market_timestamp}
  , market_price_{market_price}
  , position_size_{position_size}
  , investment_{investment}
  , entry_price_{entry_price}
  , total_entry_fees_{total_entry_fees}
  , stop_price_{stop_price}
  , stop_loss_price_{stop_loss_price}
  , take_profit_levels_{std::move(take_profit_levels)}
  , signal_exit_states_{std::move(signal_exit_states)}
  , risk_distance_{risk_distance}
  , risk_reference_price_{risk_reference_price}
  , risk_boundary_price_{risk_boundary_price}
  {
  }

  auto operator==(const OpenPositionSnapshot&) const noexcept -> bool = default;

  auto trade_id(this const OpenPositionSnapshot& self) noexcept -> std::size_t
  {
    return self.trade_id_;
  }

  auto entry_timestamp(this const OpenPositionSnapshot& self) noexcept
   -> std::time_t
  {
    return self.entry_timestamp_;
  }

  auto market_timestamp(this const OpenPositionSnapshot& self) noexcept
   -> std::time_t
  {
    return self.market_timestamp_;
  }

  auto market_price(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.market_price_;
  }

  auto position_size(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.position_size_;
  }

  auto investment(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.investment_;
  }

  auto entry_price(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.entry_price_;
  }

  auto total_entry_fees(this const OpenPositionSnapshot& self) noexcept
   -> double
  {
    return self.total_entry_fees_;
  }

  auto stop_price(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.stop_price_;
  }

  auto stop_loss_price(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.stop_loss_price_;
  }

  auto take_profit_levels(this const OpenPositionSnapshot& self) noexcept
   -> const std::vector<TakeProfitLevel>&
  {
    return self.take_profit_levels_;
  }

  auto signal_exit_states(this const OpenPositionSnapshot& self) noexcept
   -> const std::vector<SignalExitState>&
  {
    return self.signal_exit_states_;
  }

  auto risk_distance(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.risk_distance_;
  }

  auto risk_reference_price(this const OpenPositionSnapshot& self) noexcept
   -> double
  {
    return self.risk_reference_price_;
  }

  auto risk_boundary_price(this const OpenPositionSnapshot& self) noexcept
   -> double
  {
    return self.risk_boundary_price_;
  }

  auto average_price(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.position_size_ ? self.investment_ / self.position_size_ : 0.0;
  }

  auto unrealized_pnl(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.position_size_ * (self.market_price_ - self.average_price());
  }

  auto duration(this const OpenPositionSnapshot& self) noexcept -> std::time_t
  {
    return self.market_timestamp_ - self.entry_timestamp_;
  }

private:
  std::size_t trade_id_{};
  std::time_t entry_timestamp_{};
  std::time_t market_timestamp_{};
  double market_price_{};
  double position_size_{};
  double investment_{};
  double entry_price_{};
  double total_entry_fees_{};
  double stop_price_{};
  double stop_loss_price_{};
  std::vector<TakeProfitLevel> take_profit_levels_;
  std::vector<SignalExitState> signal_exit_states_;
  double risk_distance_{NAN};
  double risk_reference_price_{NAN};
  double risk_boundary_price_{NAN};
};

} // namespace pludux::backtest
