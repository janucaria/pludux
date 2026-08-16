module;

#include <cstddef>
#include <ctime>
#include <utility>
#include <vector>

export module pludux.backtest:model_closed_position;

import :model_intent;

export namespace pludux::backtest {

class ModelClosedPosition {
public:
  ModelClosedPosition() = default;

  ModelClosedPosition(std::size_t model_trade_id,
                          ModelDirection direction,
                         std::time_t entry_timestamp,
                         std::time_t exit_timestamp,
                         double normalized_entry_quantity,
                         double normalized_entry_notional,
                         double directional_price_pnl,
                          std::vector<ModelIntent> intents) noexcept
  : model_trade_id_{model_trade_id}
  , direction_{direction}
  , entry_timestamp_{entry_timestamp}
  , exit_timestamp_{exit_timestamp}
  , normalized_entry_quantity_{normalized_entry_quantity}
  , normalized_entry_notional_{normalized_entry_notional}
  , directional_price_pnl_{directional_price_pnl}
  , intents_{std::move(intents)}
  {
  }

  auto operator==(const ModelClosedPosition&) const noexcept
   -> bool = default;

  auto model_trade_id(this const ModelClosedPosition& self) noexcept
   -> std::size_t
  {
    return self.model_trade_id_;
  }

  auto direction(this const ModelClosedPosition& self) noexcept
   -> ModelDirection
  {
    return self.direction_;
  }

  auto entry_timestamp(this const ModelClosedPosition& self) noexcept
   -> std::time_t
  {
    return self.entry_timestamp_;
  }

  auto exit_timestamp(this const ModelClosedPosition& self) noexcept
   -> std::time_t
  {
    return self.exit_timestamp_;
  }

  auto duration(this const ModelClosedPosition& self) noexcept -> std::time_t
  {
    return self.exit_timestamp_ - self.entry_timestamp_;
  }

  auto
  normalized_entry_quantity(this const ModelClosedPosition& self) noexcept
   -> double
  {
    return self.normalized_entry_quantity_;
  }

  auto
  normalized_entry_notional(this const ModelClosedPosition& self) noexcept
   -> double
  {
    return self.normalized_entry_notional_;
  }

  auto directional_price_pnl(this const ModelClosedPosition& self) noexcept
   -> double
  {
    return self.directional_price_pnl_;
  }

  auto price_pnl(this const ModelClosedPosition& self) noexcept -> double
  {
    return self.normalized_entry_quantity_ > 0.0
            ? self.directional_price_pnl_ / self.normalized_entry_quantity_
            : 0.0;
  }

  auto return_ratio(this const ModelClosedPosition& self) noexcept -> double
  {
    return self.normalized_entry_notional_ > 0.0
            ? self.directional_price_pnl_ / self.normalized_entry_notional_
            : 0.0;
  }

  auto intents(this const ModelClosedPosition& self) noexcept
   -> const std::vector<ModelIntent>&
  {
    return self.intents_;
  }

private:
  std::size_t model_trade_id_{};
  ModelDirection direction_{ModelDirection::Long};
  std::time_t entry_timestamp_{};
  std::time_t exit_timestamp_{};
  double normalized_entry_quantity_{};
  double normalized_entry_notional_{};
  double directional_price_pnl_{};
  std::vector<ModelIntent> intents_{};
};

} // namespace pludux::backtest
