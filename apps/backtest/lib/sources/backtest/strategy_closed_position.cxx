module;

#include <cstddef>
#include <ctime>
#include <utility>
#include <vector>

export module pludux.backtest:strategy_closed_position;

import :strategy_intent;

export namespace pludux::backtest {

class StrategyClosedPosition {
public:
  StrategyClosedPosition() = default;

  StrategyClosedPosition(std::size_t strategy_trade_id,
                         StrategyDirection direction,
                         std::time_t entry_timestamp,
                         std::time_t exit_timestamp,
                         double normalized_entry_quantity,
                         double normalized_entry_notional,
                         double directional_price_pnl,
                         std::vector<StrategyIntent> intents) noexcept
  : strategy_trade_id_{strategy_trade_id}
  , direction_{direction}
  , entry_timestamp_{entry_timestamp}
  , exit_timestamp_{exit_timestamp}
  , normalized_entry_quantity_{normalized_entry_quantity}
  , normalized_entry_notional_{normalized_entry_notional}
  , directional_price_pnl_{directional_price_pnl}
  , intents_{std::move(intents)}
  {
  }

  auto operator==(const StrategyClosedPosition&) const noexcept
   -> bool = default;

  auto strategy_trade_id(this const StrategyClosedPosition& self) noexcept
   -> std::size_t
  {
    return self.strategy_trade_id_;
  }

  auto direction(this const StrategyClosedPosition& self) noexcept
   -> StrategyDirection
  {
    return self.direction_;
  }

  auto entry_timestamp(this const StrategyClosedPosition& self) noexcept
   -> std::time_t
  {
    return self.entry_timestamp_;
  }

  auto exit_timestamp(this const StrategyClosedPosition& self) noexcept
   -> std::time_t
  {
    return self.exit_timestamp_;
  }

  auto duration(this const StrategyClosedPosition& self) noexcept -> std::time_t
  {
    return self.exit_timestamp_ - self.entry_timestamp_;
  }

  auto
  normalized_entry_quantity(this const StrategyClosedPosition& self) noexcept
   -> double
  {
    return self.normalized_entry_quantity_;
  }

  auto
  normalized_entry_notional(this const StrategyClosedPosition& self) noexcept
   -> double
  {
    return self.normalized_entry_notional_;
  }

  auto directional_price_pnl(this const StrategyClosedPosition& self) noexcept
   -> double
  {
    return self.directional_price_pnl_;
  }

  auto price_pnl(this const StrategyClosedPosition& self) noexcept -> double
  {
    return self.normalized_entry_quantity_ > 0.0
            ? self.directional_price_pnl_ / self.normalized_entry_quantity_
            : 0.0;
  }

  auto return_ratio(this const StrategyClosedPosition& self) noexcept -> double
  {
    return self.normalized_entry_notional_ > 0.0
            ? self.directional_price_pnl_ / self.normalized_entry_notional_
            : 0.0;
  }

  auto intents(this const StrategyClosedPosition& self) noexcept
   -> const std::vector<StrategyIntent>&
  {
    return self.intents_;
  }

private:
  std::size_t strategy_trade_id_{};
  StrategyDirection direction_{StrategyDirection::Long};
  std::time_t entry_timestamp_{};
  std::time_t exit_timestamp_{};
  double normalized_entry_quantity_{};
  double normalized_entry_notional_{};
  double directional_price_pnl_{};
  std::vector<StrategyIntent> intents_{};
};

} // namespace pludux::backtest
