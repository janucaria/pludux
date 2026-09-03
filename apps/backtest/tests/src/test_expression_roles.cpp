#include <gtest/gtest.h>

#include <concepts>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

template<typename TContext, typename TNode>
concept ConvertsForContext = requires(const TNode& node,
                                      NodeToErasedMethodContext& context) {
  node_to_erased_method<TContext>(node, context);
};

static_assert(std::same_as<ModelNode, ErasedNode<BacktestMethodContext>>);
static_assert(
 std::same_as<ModelMethod, ErasedSeriesMethod<BacktestMethodContext>>);
static_assert(
 std::same_as<ComparatorNode, ErasedNode<RequestedOrderMethodContext>>);
static_assert(
 std::same_as<ComparatorMethod, ErasedSeriesMethod<RequestedOrderMethodContext>>);
static_assert(
 std::same_as<EntryFilterNode, ErasedNode<EntryFilterMethodContext>>);
static_assert(
 std::same_as<EntryFilterMethod, ErasedSeriesMethod<EntryFilterMethodContext>>);

static_assert(std::constructible_from<ComparatorNode, ValueNode>);
static_assert(!std::constructible_from<ComparatorNode, TrueNode>);
static_assert(!std::constructible_from<ComparatorNode, FalseNode>);
static_assert(std::constructible_from<ComparatorNode, DataNode>);
static_assert(std::constructible_from<ComparatorNode,
                                      LookbackNode<RequestedOrderMethodContext>>);
static_assert(std::constructible_from<
              ComparatorNode,
              AddNode<RequestedOrderMethodContext>>);
static_assert(!std::constructible_from<ComparatorNode, SeriesNode>);
static_assert(!std::constructible_from<
              ComparatorNode,
              AtrNode<RequestedOrderMethodContext>>);
static_assert(!std::constructible_from<
              ComparatorNode,
              PositionRMultipleNode<RequestedOrderMethodContext>>);
static_assert(!std::constructible_from<
              ComparatorNode,
              RiskDistanceAtrNode<RequestedOrderMethodContext>>);
static_assert(!std::constructible_from<
              ComparatorNode,
              SlRMultipleNode<RequestedOrderMethodContext>>);
static_assert(
 !std::constructible_from<ComparatorNode, PyramidingLayerNode>);
static_assert(
 !std::constructible_from<ModelNode, RequestedOrderPriceNode>);
static_assert(
 !ConvertsForContext<RequestedOrderMethodContext,
                     AtrNode<RequestedOrderMethodContext>>);
static_assert(
 !ConvertsForContext<BacktestMethodContext, RequestedOrderPriceNode>);

TEST(ExpressionRoles, BindRoleOwnedExpressionsToTheirMethodContexts)
{
  SUCCEED();
}
