module;

#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>

export module pludux.backtest:requested_order;

import :trade_entry;

export namespace pludux::backtest {

class RequestedOrder {
public:
  RequestedOrder(TradeEntry entry,
                 bool pyramiding,
                 std::optional<double> raw_requested_quantity,
                 std::optional<double> raw_requested_limit,
                 std::optional<double> drawdown_adjusted_quantity,
                 std::optional<double> drawdown_adjusted_limit,
                 double risk_distance,
                 double estimated_entry_fee,
                 double estimated_exit_fee,
                 std::optional<double> frozen_unit_quantity = std::nullopt)
  : entry_{std::move(entry)}
  , pyramiding_{pyramiding}
  , raw_requested_quantity_{raw_requested_quantity}
  , raw_requested_limit_{raw_requested_limit}
  , drawdown_adjusted_quantity_{drawdown_adjusted_quantity}
  , drawdown_adjusted_limit_{drawdown_adjusted_limit}
  , requested_quantity_{std::abs(entry_.position_size())}
  , requested_notional_{entry_.notional_value()}
  , estimated_entry_fee_{estimated_entry_fee}
  , estimated_exit_fee_{estimated_exit_fee}
  , requested_cost_{requested_notional_ + estimated_entry_fee_}
  , risk_distance_{risk_distance}
  , requested_price_risk_{requested_quantity_ * risk_distance_}
  , requested_risk_with_fees_{requested_price_risk_ + estimated_entry_fee_ +
                              estimated_exit_fee_}
  , frozen_unit_quantity_{frozen_unit_quantity}
  {
    validate();
  }

  auto operator==(const RequestedOrder&) const noexcept -> bool = default;

  auto entry(this const RequestedOrder& self) noexcept -> const TradeEntry&
  {
    return self.entry_;
  }

  auto direction(this const RequestedOrder& self) noexcept -> double
  {
    return self.entry_.is_long_direction() ? 1.0 : -1.0;
  }

  auto price(this const RequestedOrder& self) noexcept -> double
  {
    return self.entry_.price();
  }

  auto pyramiding(this const RequestedOrder& self) noexcept -> bool
  {
    return self.pyramiding_;
  }

  auto raw_requested_quantity(this const RequestedOrder& self) noexcept
   -> std::optional<double>
  {
    return self.raw_requested_quantity_;
  }

  auto raw_requested_limit(this const RequestedOrder& self) noexcept
   -> std::optional<double>
  {
    return self.raw_requested_limit_;
  }

  auto drawdown_adjusted_quantity(this const RequestedOrder& self) noexcept
   -> std::optional<double>
  {
    return self.drawdown_adjusted_quantity_;
  }

  auto drawdown_adjusted_limit(this const RequestedOrder& self) noexcept
   -> std::optional<double>
  {
    return self.drawdown_adjusted_limit_;
  }

  auto requested_quantity(this const RequestedOrder& self) noexcept -> double
  {
    return self.requested_quantity_;
  }

  auto requested_notional(this const RequestedOrder& self) noexcept -> double
  {
    return self.requested_notional_;
  }

  auto estimated_entry_fee(this const RequestedOrder& self) noexcept -> double
  {
    return self.estimated_entry_fee_;
  }

  auto estimated_exit_fee(this const RequestedOrder& self) noexcept -> double
  {
    return self.estimated_exit_fee_;
  }

  auto requested_cost(this const RequestedOrder& self) noexcept -> double
  {
    return self.requested_cost_;
  }

  auto risk_distance(this const RequestedOrder& self) noexcept -> double
  {
    return self.risk_distance_;
  }

  auto requested_price_risk(this const RequestedOrder& self) noexcept -> double
  {
    return self.requested_price_risk_;
  }

  auto requested_risk_with_fees(this const RequestedOrder& self) noexcept
   -> double
  {
    return self.requested_risk_with_fees_;
  }

  auto frozen_unit_quantity(this const RequestedOrder& self) noexcept
   -> std::optional<double>
  {
    return self.frozen_unit_quantity_;
  }

private:
  TradeEntry entry_;
  bool pyramiding_{};
  std::optional<double> raw_requested_quantity_;
  std::optional<double> raw_requested_limit_;
  std::optional<double> drawdown_adjusted_quantity_;
  std::optional<double> drawdown_adjusted_limit_;
  double requested_quantity_{};
  double requested_notional_{};
  double estimated_entry_fee_{};
  double estimated_exit_fee_{};
  double requested_cost_{};
  double risk_distance_{};
  double requested_price_risk_{};
  double requested_risk_with_fees_{};
  std::optional<double> frozen_unit_quantity_;

  static void validate_optional_non_negative(std::optional<double> value,
                                             const char* message)
  {
    if(value && (!std::isfinite(*value) || *value < 0.0)) {
      throw std::invalid_argument{message};
    }
  }

  void validate(this const RequestedOrder& self)
  {
    if(!std::isfinite(self.entry_.position_size()) ||
       self.entry_.position_size() == 0.0) {
      throw std::invalid_argument{
       "Requested order quantity must be finite and non-zero"};
    }
    if(!std::isfinite(self.entry_.price()) || self.entry_.price() <= 0.0) {
      throw std::invalid_argument{
       "Requested order price must be finite and positive"};
    }
    validate_optional_non_negative(self.raw_requested_quantity_,
                                   "Invalid raw requested quantity");
    validate_optional_non_negative(self.raw_requested_limit_,
                                   "Invalid raw requested limit");
    validate_optional_non_negative(self.drawdown_adjusted_quantity_,
                                   "Invalid drawdown-adjusted quantity");
    validate_optional_non_negative(self.drawdown_adjusted_limit_,
                                   "Invalid drawdown-adjusted limit");
    if(!std::isfinite(self.risk_distance_) || self.risk_distance_ <= 0.0) {
      throw std::invalid_argument{
       "Requested order risk distance must be finite and positive"};
    }
    if(!std::isfinite(self.estimated_entry_fee_) ||
       self.estimated_entry_fee_ < 0.0 ||
       !std::isfinite(self.estimated_exit_fee_) ||
       self.estimated_exit_fee_ < 0.0) {
      throw std::invalid_argument{
       "Requested order fees must be finite and non-negative"};
    }
    if(self.pyramiding_ != self.frozen_unit_quantity_.has_value()) {
      throw std::invalid_argument{
       "Pyramiding requested orders require a frozen Unit quantity"};
    }
    if(self.frozen_unit_quantity_ &&
       (!std::isfinite(*self.frozen_unit_quantity_) ||
        *self.frozen_unit_quantity_ <= 0.0)) {
      throw std::invalid_argument{"Invalid frozen Unit quantity"};
    }
  }
};

} // namespace pludux::backtest
