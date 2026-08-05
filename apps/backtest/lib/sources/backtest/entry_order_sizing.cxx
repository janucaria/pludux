module;

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>
#include <variant>

export module pludux.backtest:entry_order_sizing;

import :broker;
import :market;
import :trade_entry;
import :trade_exit;

export namespace pludux::backtest {

struct NearestQuantityConstraint {
  auto operator==(const NearestQuantityConstraint&) const noexcept
   -> bool = default;
};

struct MaximumQuantityConstraint {
  auto operator==(const MaximumQuantityConstraint&) const noexcept
   -> bool = default;
};

struct EntryCostBudgetConstraint {
  double budget{};

  auto operator==(const EntryCostBudgetConstraint&) const noexcept
   -> bool = default;
};

struct RiskBudgetConstraint {
  double budget{};
  double boundary_price{};

  auto operator==(const RiskBudgetConstraint&) const noexcept -> bool = default;
};

using EntryOrderSizingConstraint = std::variant<NearestQuantityConstraint,
                                                MaximumQuantityConstraint,
                                                EntryCostBudgetConstraint,
                                                RiskBudgetConstraint>;

auto entry_order_sizing_limit(const EntryOrderSizingConstraint& constraint)
 -> std::optional<double>
{
  if(const auto* budget = std::get_if<EntryCostBudgetConstraint>(&constraint)) {
    return budget->budget;
  }
  if(const auto* risk = std::get_if<RiskBudgetConstraint>(&constraint)) {
    return risk->budget;
  }
  return std::nullopt;
}

auto scale_entry_order_sizing_constraint(
 const EntryOrderSizingConstraint& constraint, double multiplier)
 -> EntryOrderSizingConstraint
{
  if(!std::isfinite(multiplier) || multiplier < 0.0) {
    throw std::invalid_argument{"Invalid sizing multiplier"};
  }
  if(const auto* budget = std::get_if<EntryCostBudgetConstraint>(&constraint)) {
    return EntryCostBudgetConstraint{budget->budget * multiplier};
  }
  if(const auto* risk = std::get_if<RiskBudgetConstraint>(&constraint)) {
    return RiskBudgetConstraint{risk->budget * multiplier,
                                risk->boundary_price};
  }
  return constraint;
}

struct EntryOrderSizingRequest {
  double requested_quantity{};
  double entry_price{};
  EntryOrderSizingConstraint constraint{NearestQuantityConstraint{}};
};

struct EntryOrderSizingResult {
  TradeEntry entry;
  double entry_fee{};
  double estimated_exit_fee{};
  double entry_cost{};
  std::optional<double> estimated_loss{};
  bool limited{};
};

namespace detail {

inline void validate_positive_finite(double value, const char* label)
{
  if(!std::isfinite(value) || value <= 0.0) {
    throw std::invalid_argument{label};
  }
}

inline auto floor_to_step(double quantity, double step) noexcept -> double
{
  if(step <= 0.0) {
    return quantity;
  }

  const auto step_count =
   std::nextafter(quantity / step, std::numeric_limits<double>::infinity());
  return step * std::floor(step_count);
}

struct CandidateCost {
  double entry_fee{};
  double estimated_exit_fee{};
  double entry_cost{};
  double estimated_loss{};
};

inline auto candidate_cost(double signed_quantity,
                           double entry_price,
                           std::optional<double> boundary_price,
                           const Broker& broker) -> CandidateCost
{
  const auto entry = TradeEntry{signed_quantity, entry_price};
  const auto entry_fee = broker.calculate_fee(entry);
  const auto entry_cost = entry.notional_value() + entry_fee;
  auto exit_fee = 0.0;
  auto estimated_loss = 0.0;

  if(boundary_price) {
    const auto exit =
     TradeExit{signed_quantity, *boundary_price, TradeExit::Reason::stop_loss};
    exit_fee = broker.calculate_fee(exit);
    estimated_loss =
     std::abs(signed_quantity) * std::abs(entry_price - *boundary_price) +
     entry_fee + exit_fee;
  }

  if(!std::isfinite(entry_fee) || entry_fee < 0.0 || !std::isfinite(exit_fee) ||
     exit_fee < 0.0 || !std::isfinite(entry_cost) || entry_cost < 0.0 ||
     !std::isfinite(estimated_loss) || estimated_loss < 0.0) {
    throw std::runtime_error{"Invalid broker cost"};
  }

  return CandidateCost{entry_fee, exit_fee, entry_cost, estimated_loss};
}

} // namespace detail

auto size_entry_order(const EntryOrderSizingRequest& request,
                      const Market& market,
                      const Broker& broker)
 -> std::optional<EntryOrderSizingResult>
{
  detail::validate_positive_finite(std::abs(request.requested_quantity),
                                   "Invalid requested quantity");
  detail::validate_positive_finite(request.entry_price, "Invalid entry price");

  const auto direction = request.requested_quantity > 0.0 ? 1.0 : -1.0;
  const auto requested_quantity = std::abs(request.requested_quantity);
  const auto quantity_step = market.quantity_step();
  const auto minimum_quantity = market.min_order_quantity();

  if(const auto* risk =
      std::get_if<RiskBudgetConstraint>(&request.constraint)) {
    detail::validate_positive_finite(risk->budget, "Invalid risk budget");
    detail::validate_positive_finite(risk->boundary_price,
                                     "Invalid risk boundary price");
    if((direction > 0.0 && risk->boundary_price >= request.entry_price) ||
       (direction < 0.0 && risk->boundary_price <= request.entry_price)) {
      throw std::invalid_argument{"Risk boundary is not adverse"};
    }
  } else if(const auto* budget =
             std::get_if<EntryCostBudgetConstraint>(&request.constraint)) {
    detail::validate_positive_finite(budget->budget, "Invalid entry budget");
  }

  auto quantity = requested_quantity;
  const auto is_nearest =
   std::holds_alternative<NearestQuantityConstraint>(request.constraint);
  if(is_nearest) {
    if(quantity_step > 0.0) {
      quantity = quantity_step * std::round(quantity / quantity_step);
    }
    if(quantity < minimum_quantity) {
      quantity = minimum_quantity;
    }
  } else {
    const auto fits = [&](double candidate_quantity) {
      const auto signed_quantity = direction * candidate_quantity;
      if(const auto* budget =
          std::get_if<EntryCostBudgetConstraint>(&request.constraint)) {
        return detail::candidate_cost(
                signed_quantity, request.entry_price, std::nullopt, broker)
                .entry_cost <= budget->budget;
      }
      if(const auto* risk =
          std::get_if<RiskBudgetConstraint>(&request.constraint)) {
        return detail::candidate_cost(signed_quantity,
                                      request.entry_price,
                                      risk->boundary_price,
                                      broker)
                .estimated_loss <= risk->budget;
      }
      return candidate_quantity <= requested_quantity;
    };

    if(!std::holds_alternative<MaximumQuantityConstraint>(request.constraint) &&
       !fits(requested_quantity)) {
      auto low = 0.0;
      auto high = requested_quantity;
      for(auto iteration = 0; iteration < 64; ++iteration) {
        const auto candidate = (low + high) * 0.5;
        if(fits(candidate)) {
          low = candidate;
        } else {
          high = candidate;
        }
      }
      quantity = low;
    }

    quantity = detail::floor_to_step(quantity, quantity_step);
    if(quantity_step > 0.0 && quantity > 0.0 && !fits(quantity)) {
      quantity = std::max(quantity - quantity_step, 0.0);
    }
    if(quantity <= 0.0 || quantity < minimum_quantity || !fits(quantity)) {
      return std::nullopt;
    }
  }

  if(quantity <= 0.0) {
    return std::nullopt;
  }

  const auto signed_quantity = direction * quantity;
  const auto risk = std::get_if<RiskBudgetConstraint>(&request.constraint);
  const auto costs = detail::candidate_cost(
   signed_quantity,
   request.entry_price,
   risk ? std::optional{risk->boundary_price} : std::nullopt,
   broker);
  return EntryOrderSizingResult{
   TradeEntry{signed_quantity, request.entry_price},
   costs.entry_fee,
   costs.estimated_exit_fee,
   costs.entry_cost,
   risk ? std::optional{costs.estimated_loss} : std::nullopt,
   quantity < requested_quantity};
}

} // namespace pludux::backtest
