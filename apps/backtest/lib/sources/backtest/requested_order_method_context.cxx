module;

#include <concepts>
#include <functional>
#include <type_traits>

export module pludux.backtest:requested_order_method_context;

import pludux;

import :requested_order;

export namespace pludux::backtest {

class RequestedOrderPriceNode;
class RequestedOrderDirectionNode;
class IsPyramidingOrderNode;
class RawRequestedQuantityNode;
class RawRequestedQuantityLimitNode;
class DrawdownAdjustedQuantityNode;
class DrawdownAdjustedQuantityLimitNode;
class RequestedQuantityNode;
class RequestedNotionalNode;
class RequestedCostNode;
class EstimatedEntryFeeNode;
class EstimatedOneRExitFeeNode;
class RequestedOrderRiskDistanceNode;
class RequestedPriceRiskNode;
class RequestedRiskWithFeesNode;
class FrozenUnitQuantityNode;

class RequestedOrderMethodContext {
public:
  explicit RequestedOrderMethodContext(const RequestedOrder& requested_order)
  : requested_order_{requested_order}
  {
  }

  auto requested_order(this const RequestedOrderMethodContext& self) noexcept
   -> const RequestedOrder&
  {
    return self.requested_order_;
  }

  template<typename TNode>
  static consteval auto node_admissible() -> bool
  {
    using Node = std::remove_cvref_t<TNode>;
    using Context = RequestedOrderMethodContext;
    return std::same_as<Node, ErasedNode<Context>> ||
           std::same_as<Node, ValueNode> || std::same_as<Node, OpenNode> ||
           std::same_as<Node, HighNode> || std::same_as<Node, LowNode> ||
           std::same_as<Node, CloseNode> || std::same_as<Node, VolumeNode> ||
           std::same_as<Node, DataNode> ||
           std::same_as<Node, LookbackNode<Context>> ||
           std::same_as<Node, BinaryOperatorNode<std::plus<>, Context>> ||
           std::same_as<Node, BinaryOperatorNode<std::minus<>, Context>> ||
           std::same_as<Node, BinaryOperatorNode<std::multiplies<>, Context>> ||
           std::same_as<Node, BinaryOperatorNode<std::divides<>, Context>> ||
           std::same_as<Node,
                        BinaryOperatorNode<AbsoluteDifference<>, Context>> ||
           std::same_as<Node, BinaryOperatorNode<Maximum<>, Context>> ||
           std::same_as<Node, BinaryOperatorNode<Minimum<>, Context>> ||
           std::same_as<Node, UnaryOperatorNode<std::negate<>, Context>> ||
           std::same_as<Node, UnaryOperatorNode<Absolute<>, Context>> ||
           std::same_as<Node, UnaryOperatorNode<SquareRoot<>, Context>> ||
           std::same_as<Node, UnaryOperatorNode<PositivePart<>, Context>> ||
           std::same_as<Node, UnaryOperatorNode<NegativePart<>, Context>> ||
           std::same_as<Node, RequestedOrderPriceNode> ||
           std::same_as<Node, RequestedOrderDirectionNode> ||
           std::same_as<Node, IsPyramidingOrderNode> ||
           std::same_as<Node, RawRequestedQuantityNode> ||
           std::same_as<Node, RawRequestedQuantityLimitNode> ||
           std::same_as<Node, DrawdownAdjustedQuantityNode> ||
           std::same_as<Node, DrawdownAdjustedQuantityLimitNode> ||
           std::same_as<Node, RequestedQuantityNode> ||
           std::same_as<Node, RequestedNotionalNode> ||
           std::same_as<Node, RequestedCostNode> ||
           std::same_as<Node, EstimatedEntryFeeNode> ||
           std::same_as<Node, EstimatedOneRExitFeeNode> ||
           std::same_as<Node, RequestedOrderRiskDistanceNode> ||
           std::same_as<Node, RequestedPriceRiskNode> ||
           std::same_as<Node, RequestedRiskWithFeesNode> ||
           std::same_as<Node, FrozenUnitQuantityNode>;
  }

private:
  const RequestedOrder& requested_order_;
};

using ComparatorNode = ErasedNode<RequestedOrderMethodContext>;
using ComparatorMethod = ErasedSeriesMethod<RequestedOrderMethodContext>;

} // namespace pludux::backtest
