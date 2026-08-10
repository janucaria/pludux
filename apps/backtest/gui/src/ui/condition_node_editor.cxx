module;

#include <array>
#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>

#include <imgui.h>

export module pludux.apps.backtest:condition_node_editor;

import pludux.backtest;

namespace pludux::backtest::gui {

export enum class ConditionNodeCapability {
  Constant,
  Comparison,
  Logical,
  Account,
  StrategyPerformance,
  RequestedOrder
};

export class ConditionNodeCatalog {
public:
  constexpr explicit ConditionNodeCatalog(
   std::span<const ConditionNodeCapability> capabilities) noexcept
  : capabilities_{capabilities}
  {
  }

  auto capabilities(this const ConditionNodeCatalog& self) noexcept
   -> std::span<const ConditionNodeCapability>
  {
    return self.capabilities_;
  }

  auto contains(this const ConditionNodeCatalog& self,
                ConditionNodeCapability capability) noexcept -> bool
  {
    for(const auto value : self.capabilities_) {
      if(value == capability) {
        return true;
      }
    }
    return false;
  }

private:
  std::span<const ConditionNodeCapability> capabilities_;
};

export inline constexpr auto entry_filter_capabilities =
 std::array{ConditionNodeCapability::Constant,
            ConditionNodeCapability::Comparison,
            ConditionNodeCapability::Logical,
            ConditionNodeCapability::Account,
            ConditionNodeCapability::StrategyPerformance,
            ConditionNodeCapability::RequestedOrder};

export inline constexpr auto entry_filter_node_catalog =
 ConditionNodeCatalog{entry_filter_capabilities};

namespace {

using EntryContext = EntryFilterMethodContext;
using EntryNode = ErasedNode<EntryContext>;

enum class BooleanNodeType {
  Always,
  Never,
  GreaterThan,
  GreaterEqual,
  LessThan,
  LessEqual,
  Equal,
  NotEqual,
  And,
  Or,
  Xor,
  Not
};

enum class ScalarNodeType {
  Value,
  Equity,
  EquityPercent,
  Drawdown,
  StrategyPerformance,
  RequestedOrderPrice,
  RequestedOrderDirection,
  IsPyramidingOrder,
  RawRequestedQuantity,
  RawRequestedQuantityLimit,
  DrawdownAdjustedQuantity,
  DrawdownAdjustedQuantityLimit,
  RequestedQuantity,
  RequestedNotional,
  RequestedCost,
  EstimatedEntryFee,
  EstimatedOneRExitFee,
  RequestedOrderRiskDistance,
  RequestedPriceRisk,
  RequestedRiskWithFees,
  FrozenUnitQuantity
};

inline constexpr auto boolean_node_labels =
 std::array{"Always",
            "Never",
            "Greater than",
            "Greater than or equal",
            "Less than",
            "Less than or equal",
            "Equal",
            "Not equal",
            "All conditions (AND)",
            "Any condition (OR)",
            "Exactly one condition (XOR)",
            "Negate condition (NOT)"};

inline constexpr auto scalar_node_labels =
 std::array{"Constant",
            "Account equity",
            "Account equity percentage",
            "Account drawdown",
            "Strategy performance",
            "Requested order price",
            "Requested order direction",
            "Is pyramiding order",
            "Raw requested quantity",
            "Raw requested quantity limit",
            "Drawdown-adjusted quantity",
            "Drawdown-adjusted quantity limit",
            "Requested quantity",
            "Requested notional",
            "Requested cost",
            "Estimated entry fee",
            "Estimated 1R exit fee",
            "Requested order risk distance",
            "Requested price risk",
            "Requested risk with fees",
            "Frozen Unit quantity"};

inline constexpr auto performance_metric_labels =
 std::array{"Lifetime completed positions",
            "Effective completed positions",
            "Frequentist win rate",
            "Frequentist break-even rate",
            "Frequentist loss rate",
            "Current winning streak",
            "Current losing streak",
            "Maximum winning streak",
            "Maximum losing streak",
            "Frequentist mean return",
            "Frequentist return standard deviation",
            "Bayesian win probability",
            "Bayesian win probability lower 95%",
            "Bayesian win probability upper 95%",
            "Bayesian winning payoff",
            "Bayesian winning payoff lower 95%",
            "Bayesian winning payoff upper 95%",
            "Bayesian losing payoff",
            "Bayesian losing payoff lower 95%",
            "Bayesian losing payoff upper 95%"};

auto boolean_node_type(const EntryNode& node) noexcept -> BooleanNodeType
{
  if(node_cast<FalseNode>(node)) {
    return BooleanNodeType::Never;
  }
  if(node_cast<GreaterThanNode<EntryContext>>(node)) {
    return BooleanNodeType::GreaterThan;
  }
  if(node_cast<GreaterEqualNode<EntryContext>>(node)) {
    return BooleanNodeType::GreaterEqual;
  }
  if(node_cast<LessThanNode<EntryContext>>(node)) {
    return BooleanNodeType::LessThan;
  }
  if(node_cast<LessEqualNode<EntryContext>>(node)) {
    return BooleanNodeType::LessEqual;
  }
  if(node_cast<EqualNode<EntryContext>>(node)) {
    return BooleanNodeType::Equal;
  }
  if(node_cast<NotEqualNode<EntryContext>>(node)) {
    return BooleanNodeType::NotEqual;
  }
  if(node_cast<LogicalAndNode<EntryContext>>(node)) {
    return BooleanNodeType::And;
  }
  if(node_cast<LogicalOrNode<EntryContext>>(node)) {
    return BooleanNodeType::Or;
  }
  if(node_cast<LogicalXorNode<EntryContext>>(node)) {
    return BooleanNodeType::Xor;
  }
  if(node_cast<LogicalNotNode<EntryContext>>(node)) {
    return BooleanNodeType::Not;
  }
  return BooleanNodeType::Always;
}

auto scalar_node_type(const EntryNode& node) noexcept -> ScalarNodeType
{
  if(node_cast<EquityNode>(node)) {
    return ScalarNodeType::Equity;
  }
  if(node_cast<EquityPercentNode>(node)) {
    return ScalarNodeType::EquityPercent;
  }
  if(node_cast<DrawdownNode>(node)) {
    return ScalarNodeType::Drawdown;
  }
  if(node_cast<StrategyPerformanceNode>(node)) {
    return ScalarNodeType::StrategyPerformance;
  }
#define PLUDUX_ENTRY_FILTER_SCALAR_TYPE(Node, Type) \
  if(node_cast<Node>(node)) {                       \
    return ScalarNodeType::Type;                    \
  }
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(RequestedOrderPriceNode, RequestedOrderPrice)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(RequestedOrderDirectionNode,
                                  RequestedOrderDirection)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(IsPyramidingOrderNode, IsPyramidingOrder)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(RawRequestedQuantityNode,
                                  RawRequestedQuantity)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(RawRequestedQuantityLimitNode,
                                  RawRequestedQuantityLimit)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(DrawdownAdjustedQuantityNode,
                                  DrawdownAdjustedQuantity)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(DrawdownAdjustedQuantityLimitNode,
                                  DrawdownAdjustedQuantityLimit)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(RequestedQuantityNode, RequestedQuantity)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(RequestedNotionalNode, RequestedNotional)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(RequestedCostNode, RequestedCost)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(EstimatedEntryFeeNode, EstimatedEntryFee)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(EstimatedOneRExitFeeNode,
                                  EstimatedOneRExitFee)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(RequestedOrderRiskDistanceNode,
                                  RequestedOrderRiskDistance)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(RequestedPriceRiskNode, RequestedPriceRisk)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(RequestedRiskWithFeesNode,
                                  RequestedRiskWithFees)
  PLUDUX_ENTRY_FILTER_SCALAR_TYPE(FrozenUnitQuantityNode, FrozenUnitQuantity)
#undef PLUDUX_ENTRY_FILTER_SCALAR_TYPE
  return ScalarNodeType::Value;
}

auto make_boolean_node(BooleanNodeType type) -> EntryNode
{
  const auto scalar = [] { return EntryNode{ValueNode{0.0}}; };
  const auto condition = [] { return EntryNode{TrueNode{}}; };
  switch(type) {
  case BooleanNodeType::Always:
    return EntryNode{TrueNode{}};
  case BooleanNodeType::Never:
    return EntryNode{FalseNode{}};
  case BooleanNodeType::GreaterThan:
    return EntryNode{GreaterThanNode<EntryContext>{scalar(), scalar()}};
  case BooleanNodeType::GreaterEqual:
    return EntryNode{GreaterEqualNode<EntryContext>{scalar(), scalar()}};
  case BooleanNodeType::LessThan:
    return EntryNode{LessThanNode<EntryContext>{scalar(), scalar()}};
  case BooleanNodeType::LessEqual:
    return EntryNode{LessEqualNode<EntryContext>{scalar(), scalar()}};
  case BooleanNodeType::Equal:
    return EntryNode{EqualNode<EntryContext>{scalar(), scalar()}};
  case BooleanNodeType::NotEqual:
    return EntryNode{NotEqualNode<EntryContext>{scalar(), scalar()}};
  case BooleanNodeType::And:
    return EntryNode{LogicalAndNode<EntryContext>{condition(), condition()}};
  case BooleanNodeType::Or:
    return EntryNode{LogicalOrNode<EntryContext>{condition(), condition()}};
  case BooleanNodeType::Xor:
    return EntryNode{LogicalXorNode<EntryContext>{condition(), condition()}};
  case BooleanNodeType::Not:
    return EntryNode{LogicalNotNode<EntryContext>{condition()}};
  }
  return EntryNode{TrueNode{}};
}

auto make_scalar_node(ScalarNodeType type) -> EntryNode
{
  switch(type) {
  case ScalarNodeType::Value:
    return EntryNode{ValueNode{0.0}};
  case ScalarNodeType::Equity:
    return EntryNode{EquityNode{}};
  case ScalarNodeType::EquityPercent:
    return EntryNode{EquityPercentNode{}};
  case ScalarNodeType::Drawdown:
    return EntryNode{DrawdownNode{}};
  case ScalarNodeType::StrategyPerformance:
    return EntryNode{
     StrategyPerformanceNode{StrategyPerformanceMetric::LifetimeCount}};
  case ScalarNodeType::RequestedOrderPrice:
    return EntryNode{RequestedOrderPriceNode{}};
  case ScalarNodeType::RequestedOrderDirection:
    return EntryNode{RequestedOrderDirectionNode{}};
  case ScalarNodeType::IsPyramidingOrder:
    return EntryNode{IsPyramidingOrderNode{}};
  case ScalarNodeType::RawRequestedQuantity:
    return EntryNode{RawRequestedQuantityNode{}};
  case ScalarNodeType::RawRequestedQuantityLimit:
    return EntryNode{RawRequestedQuantityLimitNode{}};
  case ScalarNodeType::DrawdownAdjustedQuantity:
    return EntryNode{DrawdownAdjustedQuantityNode{}};
  case ScalarNodeType::DrawdownAdjustedQuantityLimit:
    return EntryNode{DrawdownAdjustedQuantityLimitNode{}};
  case ScalarNodeType::RequestedQuantity:
    return EntryNode{RequestedQuantityNode{}};
  case ScalarNodeType::RequestedNotional:
    return EntryNode{RequestedNotionalNode{}};
  case ScalarNodeType::RequestedCost:
    return EntryNode{RequestedCostNode{}};
  case ScalarNodeType::EstimatedEntryFee:
    return EntryNode{EstimatedEntryFeeNode{}};
  case ScalarNodeType::EstimatedOneRExitFee:
    return EntryNode{EstimatedOneRExitFeeNode{}};
  case ScalarNodeType::RequestedOrderRiskDistance:
    return EntryNode{RequestedOrderRiskDistanceNode{}};
  case ScalarNodeType::RequestedPriceRisk:
    return EntryNode{RequestedPriceRiskNode{}};
  case ScalarNodeType::RequestedRiskWithFees:
    return EntryNode{RequestedRiskWithFeesNode{}};
  case ScalarNodeType::FrozenUnitQuantity:
    return EntryNode{FrozenUnitQuantityNode{}};
  }
  return EntryNode{ValueNode{0.0}};
}

auto render_scalar_node(EntryNode& node, int& next_id) -> bool;
auto render_boolean_node(EntryNode& node, int& next_id) -> bool;

template<typename TComparison>
auto render_comparison(EntryNode& node, int& next_id) -> bool
{
  const auto* comparison = node_cast<TComparison>(node);
  if(!comparison) {
    return false;
  }

  auto target = comparison->target();
  auto threshold = comparison->threshold();
  auto changed = false;

  ImGui::Indent();
  ImGui::TextUnformatted("Left operand");
  changed |= render_scalar_node(target, next_id);
  ImGui::TextUnformatted("Right operand");
  changed |= render_scalar_node(threshold, next_id);
  ImGui::Unindent();

  if(changed) {
    node = EntryNode{TComparison{std::move(target), std::move(threshold)}};
  }
  return changed;
}

template<typename TLogical>
auto render_binary_logical(EntryNode& node, int& next_id) -> bool
{
  const auto* logical = node_cast<TLogical>(node);
  if(!logical) {
    return false;
  }

  auto first = logical->first_condition();
  auto second = logical->second_condition();
  auto changed = false;

  ImGui::Indent();
  ImGui::TextUnformatted("First condition");
  changed |= render_boolean_node(first, next_id);
  ImGui::TextUnformatted("Second condition");
  changed |= render_boolean_node(second, next_id);
  ImGui::Unindent();

  if(changed) {
    node = EntryNode{TLogical{std::move(first), std::move(second)}};
  }
  return changed;
}

auto render_scalar_node(EntryNode& node, int& next_id) -> bool
{
  const auto node_id = next_id++;
  ImGui::PushID(node_id);

  auto type = scalar_node_type(node);
  auto type_index = static_cast<int>(type);
  auto changed = ImGui::Combo("Source",
                              &type_index,
                              scalar_node_labels.data(),
                              static_cast<int>(scalar_node_labels.size()));
  if(changed) {
    type = static_cast<ScalarNodeType>(type_index);
    node = make_scalar_node(type);
  }

  switch(type) {
  case ScalarNodeType::Value: {
    auto value = node_cast<ValueNode>(node)->value();
    if(ImGui::InputDouble("Value", &value, 0.01, 0.1, "%.6f")) {
      node = EntryNode{ValueNode{value}};
      changed = true;
    }
    break;
  }
  case ScalarNodeType::StrategyPerformance: {
    const auto* performance = node_cast<StrategyPerformanceNode>(node);
    auto metric_index = static_cast<int>(performance->metric());
    const auto metric_changed =
     ImGui::Combo("Metric",
                  &metric_index,
                  performance_metric_labels.data(),
                  static_cast<int>(performance_metric_labels.size()));
    if(metric_changed) {
      node = EntryNode{StrategyPerformanceNode{
       static_cast<StrategyPerformanceMetric>(metric_index)}};
      changed = true;
    }
    break;
  }
  case ScalarNodeType::Equity:
  case ScalarNodeType::EquityPercent:
  case ScalarNodeType::Drawdown:
  case ScalarNodeType::RequestedOrderPrice:
  case ScalarNodeType::RequestedOrderDirection:
  case ScalarNodeType::IsPyramidingOrder:
  case ScalarNodeType::RawRequestedQuantity:
  case ScalarNodeType::RawRequestedQuantityLimit:
  case ScalarNodeType::DrawdownAdjustedQuantity:
  case ScalarNodeType::DrawdownAdjustedQuantityLimit:
  case ScalarNodeType::RequestedQuantity:
  case ScalarNodeType::RequestedNotional:
  case ScalarNodeType::RequestedCost:
  case ScalarNodeType::EstimatedEntryFee:
  case ScalarNodeType::EstimatedOneRExitFee:
  case ScalarNodeType::RequestedOrderRiskDistance:
  case ScalarNodeType::RequestedPriceRisk:
  case ScalarNodeType::RequestedRiskWithFees:
  case ScalarNodeType::FrozenUnitQuantity:
    break;
  }

  ImGui::PopID();
  return changed;
}

auto render_boolean_node(EntryNode& node, int& next_id) -> bool
{
  const auto node_id = next_id++;
  ImGui::PushID(node_id);

  auto type = boolean_node_type(node);
  auto type_index = static_cast<int>(type);
  auto changed = ImGui::Combo("Rule",
                              &type_index,
                              boolean_node_labels.data(),
                              static_cast<int>(boolean_node_labels.size()));
  if(changed) {
    type = static_cast<BooleanNodeType>(type_index);
    node = make_boolean_node(type);
  }

  switch(type) {
  case BooleanNodeType::GreaterThan:
    changed |= render_comparison<GreaterThanNode<EntryContext>>(node, next_id);
    break;
  case BooleanNodeType::GreaterEqual:
    changed |= render_comparison<GreaterEqualNode<EntryContext>>(node, next_id);
    break;
  case BooleanNodeType::LessThan:
    changed |= render_comparison<LessThanNode<EntryContext>>(node, next_id);
    break;
  case BooleanNodeType::LessEqual:
    changed |= render_comparison<LessEqualNode<EntryContext>>(node, next_id);
    break;
  case BooleanNodeType::Equal:
    changed |= render_comparison<EqualNode<EntryContext>>(node, next_id);
    break;
  case BooleanNodeType::NotEqual:
    changed |= render_comparison<NotEqualNode<EntryContext>>(node, next_id);
    break;
  case BooleanNodeType::And:
    changed |=
     render_binary_logical<LogicalAndNode<EntryContext>>(node, next_id);
    break;
  case BooleanNodeType::Or:
    changed |=
     render_binary_logical<LogicalOrNode<EntryContext>>(node, next_id);
    break;
  case BooleanNodeType::Xor:
    changed |=
     render_binary_logical<LogicalXorNode<EntryContext>>(node, next_id);
    break;
  case BooleanNodeType::Not: {
    const auto* logical = node_cast<LogicalNotNode<EntryContext>>(node);
    auto condition = logical->other_condition();
    ImGui::Indent();
    ImGui::TextUnformatted("Condition");
    if(render_boolean_node(condition, next_id)) {
      node = EntryNode{LogicalNotNode<EntryContext>{std::move(condition)}};
      changed = true;
    }
    ImGui::Unindent();
    break;
  }
  case BooleanNodeType::Always:
  case BooleanNodeType::Never:
    break;
  }

  ImGui::PopID();
  return changed;
}

} // namespace

export auto entry_filter_node_editor(ErasedNode<EntryFilterMethodContext>& root)
 -> bool
{
  auto next_id = 0;
  return render_boolean_node(root, next_id);
}

} // namespace pludux::backtest::gui
