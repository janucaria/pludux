#include <gtest/gtest.h>

#include <cmath>
#include <optional>
#include <stdexcept>
#include <vector>

import pludux.backtest;

namespace pludux::backtest::tests {
namespace {

auto make_snapshot() -> AssetSnapshot
{
  static const auto asset = Asset{"Test",
                                  AssetHistory{{"Datetime", {1.0}},
                                               {"Open", {100.0}},
                                               {"High", {100.0}},
                                               {"Low", {100.0}},
                                               {"Close", {100.0}},
                                               {"Volume", {0.0}}}};
  return asset.get_snapshot(0);
}

auto make_initial_order() -> RequestedOrder
{
  return RequestedOrder{
   TradeEntry{8.0, 100.0}, false, 10.0, 1'000.0, 8.4, 800.0, 5.0, 2.0, 3.0};
}

} // namespace

TEST(RequestedOrder, DerivesImmutableOrderValues)
{
  const auto order = make_initial_order();

  EXPECT_DOUBLE_EQ(order.direction(), 1.0);
  EXPECT_DOUBLE_EQ(order.price(), 100.0);
  EXPECT_FALSE(order.pyramiding());
  EXPECT_EQ(order.raw_requested_quantity(), 10.0);
  EXPECT_EQ(order.raw_requested_limit(), 1'000.0);
  EXPECT_EQ(order.drawdown_adjusted_quantity(), 8.4);
  EXPECT_EQ(order.drawdown_adjusted_limit(), 800.0);
  EXPECT_DOUBLE_EQ(order.requested_quantity(), 8.0);
  EXPECT_DOUBLE_EQ(order.requested_notional(), 800.0);
  EXPECT_DOUBLE_EQ(order.estimated_entry_fee(), 2.0);
  EXPECT_DOUBLE_EQ(order.estimated_exit_fee(), 3.0);
  EXPECT_DOUBLE_EQ(order.requested_cost(), 802.0);
  EXPECT_DOUBLE_EQ(order.risk_distance(), 5.0);
  EXPECT_DOUBLE_EQ(order.requested_price_risk(), 40.0);
  EXPECT_DOUBLE_EQ(order.requested_risk_with_fees(), 45.0);
  EXPECT_FALSE(order.frozen_unit_quantity());
  EXPECT_DOUBLE_EQ(order.entry().position_size(), 8.0);
}

TEST(RequestedOrder, KeepsShortQuantityAbsoluteAndDirectionSeparate)
{
  const auto order = RequestedOrder{TradeEntry{-7.0, 90.0},
                                    true,
                                    std::nullopt,
                                    std::nullopt,
                                    std::nullopt,
                                    std::nullopt,
                                    4.0,
                                    1.0,
                                    2.0,
                                    8.0};

  EXPECT_DOUBLE_EQ(order.direction(), -1.0);
  EXPECT_DOUBLE_EQ(order.requested_quantity(), 7.0);
  EXPECT_DOUBLE_EQ(order.requested_notional(), 630.0);
  EXPECT_EQ(order.frozen_unit_quantity(), 8.0);
}

TEST(RequestedOrder, RejectsInvalidConstruction)
{
  EXPECT_THROW((RequestedOrder{TradeEntry{0.0, 100.0},
                               false,
                               1.0,
                               std::nullopt,
                               1.0,
                               std::nullopt,
                               5.0,
                               0.0,
                               0.0}),
               std::invalid_argument);
  EXPECT_THROW((RequestedOrder{TradeEntry{1.0, 100.0},
                               true,
                               std::nullopt,
                               std::nullopt,
                               std::nullopt,
                               std::nullopt,
                               5.0,
                               0.0,
                               0.0}),
               std::invalid_argument);
}

TEST(RequestedOrderMethod, ReadsOnlyRequestedOrderValues)
{
  const auto order = make_initial_order();
  const auto context = RequestedOrderMethodContext{order};
  const auto snapshot = make_snapshot();

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(
    RequestedOrderValueMethod{RequestedOrderValue::RequestedNotional},
    snapshot,
    context),
   800.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(
    RequestedOrderValueMethod{RequestedOrderValue::RequestedRiskWithFees},
    snapshot,
    context),
   45.0);
  EXPECT_TRUE(std::isnan(evaluate_series_method(
   RequestedOrderValueMethod{RequestedOrderValue::FrozenUnitQuantity},
   snapshot,
   context)));
}

TEST(RequestedOrderNode, ConvertsForRequestedOrderContext)
{
  auto conversion_context = NodeToErasedMethodContext{};
  const auto method = node_to_erased_method<RequestedOrderMethodContext>(
   RequestedNotionalNode{}, conversion_context);
  const auto order = make_initial_order();

  EXPECT_DOUBLE_EQ(evaluate_series_method(method,
                                          make_snapshot(),
                                          RequestedOrderMethodContext{order}),
                   800.0);
}

TEST(RequestedOrderNode, CombinesOrderValuesWithBasicMath)
{
  auto conversion_context = NodeToErasedMethodContext{};
  const auto method = node_to_erased_method<ErasedSeriesMethodContext>(
   DivideNode{AddNode{RequestedNotionalNode{}, RequestedPriceRiskNode{}},
              ValueNode{2.0}},
   conversion_context);
  const auto order = make_initial_order();

  EXPECT_DOUBLE_EQ(evaluate_series_method(method,
                                          make_snapshot(),
                                          ErasedSeriesMethodContext{
                                           RequestedOrderMethodContext{order}}),
                   420.0);
}

TEST(RequestedOrderNode, DedicatedParserRoundTripsEveryLeaf)
{
  const auto nodes = std::vector<ErasedNode<ErasedSeriesMethodContext>>{
   RequestedOrderPriceNode{},
   RequestedOrderDirectionNode{},
   IsPyramidingOrderNode{},
   RawRequestedQuantityNode{},
   RawRequestedQuantityLimitNode{},
   DrawdownAdjustedQuantityNode{},
   DrawdownAdjustedQuantityLimitNode{},
   RequestedQuantityNode{},
   RequestedNotionalNode{},
   RequestedCostNode{},
   EstimatedEntryFeeNode{},
   EstimatedOneRExitFeeNode{},
   RequestedOrderRiskDistanceNode{},
   RequestedPriceRiskNode{},
   RequestedRiskWithFeesNode{},
   FrozenUnitQuantityNode{}};
  auto parser = make_requested_order_config_parser();

  for(const auto& node : nodes) {
    const auto json = parser.serialize_node(node);
    ASSERT_FALSE(json.is_null());
    EXPECT_EQ(parser.parse_node(json), node);
  }
}

TEST(RequestedOrderNode, RejectsNestedMarketExpression)
{
  EXPECT_THROW(
   (PortfolioEntryComparator{AddNode{RequestedQuantityNode{}, CloseNode{}},
                             PortfolioEntryComparatorOrder::HigherFirst}),
   std::invalid_argument);
}

} // namespace pludux::backtest::tests
