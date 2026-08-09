module;

#include <cstddef>
#include <functional>
#include <limits>
#include <string_view>
#include <type_traits>

export module pludux.backtest:requested_order_method;

import pludux;

import :requested_order_method_context;

export namespace pludux::backtest {

enum class RequestedOrderValue {
  Price,
  Direction,
  IsPyramiding,
  RawRequestedQuantity,
  RawRequestedQuantityLimit,
  DrawdownAdjustedQuantity,
  DrawdownAdjustedQuantityLimit,
  RequestedQuantity,
  RequestedNotional,
  RequestedCost,
  EstimatedEntryFee,
  EstimatedExitFee,
  RiskDistance,
  RequestedPriceRisk,
  RequestedRiskWithFees,
  FrozenUnitQuantity
};

class RequestedOrderValueMethod {
public:
  explicit RequestedOrderValueMethod(RequestedOrderValue value) noexcept
  : value_{value}
  {
  }

  auto operator==(const RequestedOrderValueMethod&) const noexcept
   -> bool = default;

  auto value(this const RequestedOrderValueMethod& self) noexcept
   -> RequestedOrderValue
  {
    return self.value_;
  }

private:
  RequestedOrderValue value_;
};

auto hash_series_method(const RequestedOrderValueMethod& method) noexcept
 -> std::size_t
{
  auto seed =
   std::hash<std::string_view>{}("pludux.backtest.RequestedOrderValueMethod");
  seed ^= std::hash<int>{}(static_cast<int>(method.value())) + 0x9e3779b9U +
          (seed << 6U) + (seed >> 2U);
  return seed;
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const RequestedOrderValueMethod& method,
                       AssetSnapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto unavailable = [] {
    return std::numeric_limits<double>::quiet_NaN();
  };
  const auto* order = [&] {
    if constexpr(requires { context.requested_order(); }) {
      return &context.requested_order();
    } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                       ErasedSeriesMethodContext>) {
      const auto* requested_order_context =
       series_method_context_cast<RequestedOrderMethodContext>(context);
      return requested_order_context
              ? &requested_order_context->requested_order()
              : static_cast<const RequestedOrder*>(nullptr);
    } else {
      return static_cast<const RequestedOrder*>(nullptr);
    }
  }();
  if(!order) {
    return unavailable();
  }
  switch(method.value()) {
  case RequestedOrderValue::Price:
    return order->price();
  case RequestedOrderValue::Direction:
    return order->direction();
  case RequestedOrderValue::IsPyramiding:
    return order->pyramiding() ? 1.0 : 0.0;
  case RequestedOrderValue::RawRequestedQuantity:
    return order->raw_requested_quantity().value_or(unavailable());
  case RequestedOrderValue::RawRequestedQuantityLimit:
    return order->raw_requested_limit().value_or(unavailable());
  case RequestedOrderValue::DrawdownAdjustedQuantity:
    return order->drawdown_adjusted_quantity().value_or(unavailable());
  case RequestedOrderValue::DrawdownAdjustedQuantityLimit:
    return order->drawdown_adjusted_limit().value_or(unavailable());
  case RequestedOrderValue::RequestedQuantity:
    return order->requested_quantity();
  case RequestedOrderValue::RequestedNotional:
    return order->requested_notional();
  case RequestedOrderValue::RequestedCost:
    return order->requested_cost();
  case RequestedOrderValue::EstimatedEntryFee:
    return order->estimated_entry_fee();
  case RequestedOrderValue::EstimatedExitFee:
    return order->estimated_exit_fee();
  case RequestedOrderValue::RiskDistance:
    return order->risk_distance();
  case RequestedOrderValue::RequestedPriceRisk:
    return order->requested_price_risk();
  case RequestedOrderValue::RequestedRiskWithFees:
    return order->requested_risk_with_fees();
  case RequestedOrderValue::FrozenUnitQuantity:
    return order->frozen_unit_quantity().value_or(unavailable());
  }
  return unavailable();
}

} // namespace pludux::backtest
