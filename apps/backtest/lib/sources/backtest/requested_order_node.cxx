module;

#include <concepts>

export module pludux.backtest:requested_order_node;

import pludux;

import :requested_order_method;

export namespace pludux::backtest {

#define PLUDUX_REQUESTED_ORDER_NODE(Name, Value)                           \
  class Name {                                                             \
  public:                                                                  \
    auto operator==(const Name&) const noexcept -> bool = default;         \
  };                                                                       \
                                                                           \
  template<typename TContext>                                              \
  auto pludux_tag_invoke(                                                  \
   NodeToErasedMethod<TContext>, const Name&, NodeToErasedMethodContext&)  \
   -> ErasedSeriesMethod<TContext>                                         \
  {                                                                        \
    return ErasedSeriesMethod<TContext>{RequestedOrderValueMethod{Value}}; \
  }

PLUDUX_REQUESTED_ORDER_NODE(RequestedOrderPriceNode, RequestedOrderValue::Price)
PLUDUX_REQUESTED_ORDER_NODE(RequestedOrderDirectionNode,
                            RequestedOrderValue::Direction)
PLUDUX_REQUESTED_ORDER_NODE(IsPyramidingOrderNode,
                            RequestedOrderValue::IsPyramiding)
PLUDUX_REQUESTED_ORDER_NODE(RawRequestedQuantityNode,
                            RequestedOrderValue::RawRequestedQuantity)
PLUDUX_REQUESTED_ORDER_NODE(RawRequestedQuantityLimitNode,
                            RequestedOrderValue::RawRequestedQuantityLimit)
PLUDUX_REQUESTED_ORDER_NODE(DrawdownAdjustedQuantityNode,
                            RequestedOrderValue::DrawdownAdjustedQuantity)
PLUDUX_REQUESTED_ORDER_NODE(DrawdownAdjustedQuantityLimitNode,
                            RequestedOrderValue::DrawdownAdjustedQuantityLimit)
PLUDUX_REQUESTED_ORDER_NODE(RequestedQuantityNode,
                            RequestedOrderValue::RequestedQuantity)
PLUDUX_REQUESTED_ORDER_NODE(RequestedNotionalNode,
                            RequestedOrderValue::RequestedNotional)
PLUDUX_REQUESTED_ORDER_NODE(RequestedCostNode,
                            RequestedOrderValue::RequestedCost)
PLUDUX_REQUESTED_ORDER_NODE(EstimatedEntryFeeNode,
                            RequestedOrderValue::EstimatedEntryFee)
PLUDUX_REQUESTED_ORDER_NODE(EstimatedOneRExitFeeNode,
                            RequestedOrderValue::EstimatedExitFee)
PLUDUX_REQUESTED_ORDER_NODE(RequestedOrderRiskDistanceNode,
                            RequestedOrderValue::RiskDistance)
PLUDUX_REQUESTED_ORDER_NODE(RequestedPriceRiskNode,
                            RequestedOrderValue::RequestedPriceRisk)
PLUDUX_REQUESTED_ORDER_NODE(RequestedRiskWithFeesNode,
                            RequestedOrderValue::RequestedRiskWithFees)
PLUDUX_REQUESTED_ORDER_NODE(FrozenUnitQuantityNode,
                            RequestedOrderValue::FrozenUnitQuantity)

#undef PLUDUX_REQUESTED_ORDER_NODE

} // namespace pludux::backtest
