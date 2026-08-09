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
  template<MethodContextable TContext>                                     \
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

auto is_requested_order_expression(
 const ErasedNode<ErasedSeriesMethodContext>& node) noexcept -> bool
{
  if(node_cast<ValueNode>(node) || node_cast<RequestedOrderPriceNode>(node) ||
     node_cast<RequestedOrderDirectionNode>(node) ||
     node_cast<IsPyramidingOrderNode>(node) ||
     node_cast<RawRequestedQuantityNode>(node) ||
     node_cast<RawRequestedQuantityLimitNode>(node) ||
     node_cast<DrawdownAdjustedQuantityNode>(node) ||
     node_cast<DrawdownAdjustedQuantityLimitNode>(node) ||
     node_cast<RequestedQuantityNode>(node) ||
     node_cast<RequestedNotionalNode>(node) ||
     node_cast<RequestedCostNode>(node) ||
     node_cast<EstimatedEntryFeeNode>(node) ||
     node_cast<EstimatedOneRExitFeeNode>(node) ||
     node_cast<RequestedOrderRiskDistanceNode>(node) ||
     node_cast<RequestedPriceRiskNode>(node) ||
     node_cast<RequestedRiskWithFeesNode>(node) ||
     node_cast<FrozenUnitQuantityNode>(node)) {
    return true;
  }
  const auto binary_valid = [&]<typename TNode> {
    if(const auto* binary = node_cast<TNode>(node)) {
      return is_requested_order_expression(binary->left()) &&
             is_requested_order_expression(binary->right());
    }
    return false;
  };
  if(binary_valid.template operator()<AddNode>() ||
     binary_valid.template operator()<SubtractNode>() ||
     binary_valid.template operator()<MultiplyNode>() ||
     binary_valid.template operator()<DivideNode>() ||
     binary_valid.template operator()<AbsDiffNode>() ||
     binary_valid.template operator()<MaxNode>() ||
     binary_valid.template operator()<MinNode>()) {
    return true;
  }
  const auto unary_valid = [&]<typename TNode> {
    if(const auto* unary = node_cast<TNode>(node)) {
      return is_requested_order_expression(unary->operand());
    }
    return false;
  };
  return unary_valid.template operator()<NegateNode>() ||
         unary_valid.template operator()<AbsNode>() ||
         unary_valid.template operator()<SqrtNode>() ||
         unary_valid.template operator()<PositivePartNode>() ||
         unary_valid.template operator()<NegativePartNode>();
}

} // namespace pludux::backtest
