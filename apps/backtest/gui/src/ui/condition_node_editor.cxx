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
  StrategyPerformance
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

export inline constexpr auto execution_filter_capabilities =
 std::array{ConditionNodeCapability::Constant,
            ConditionNodeCapability::Comparison,
            ConditionNodeCapability::Logical,
            ConditionNodeCapability::Account,
            ConditionNodeCapability::StrategyPerformance};

export inline constexpr auto execution_filter_node_catalog =
 ConditionNodeCatalog{execution_filter_capabilities};

namespace {

using ExecutionContext = ExecutionFilterMethodContext;
using ExecutionNode = ErasedNode<ExecutionContext>;

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
  StrategyPerformance
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
            "Strategy performance"};

inline constexpr auto performance_metric_labels =
 std::array{"Lifetime completed positions",
            "Effective completed positions",
            "Frequentist win rate",
            "Frequentist break-even rate",
            "Frequentist loss rate",
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

auto boolean_node_type(const ExecutionNode& node) noexcept -> BooleanNodeType
{
  if(node_cast<FalseNode>(node)) {
    return BooleanNodeType::Never;
  }
  if(node_cast<GreaterThanNode<ExecutionContext>>(node)) {
    return BooleanNodeType::GreaterThan;
  }
  if(node_cast<GreaterEqualNode<ExecutionContext>>(node)) {
    return BooleanNodeType::GreaterEqual;
  }
  if(node_cast<LessThanNode<ExecutionContext>>(node)) {
    return BooleanNodeType::LessThan;
  }
  if(node_cast<LessEqualNode<ExecutionContext>>(node)) {
    return BooleanNodeType::LessEqual;
  }
  if(node_cast<EqualNode<ExecutionContext>>(node)) {
    return BooleanNodeType::Equal;
  }
  if(node_cast<NotEqualNode<ExecutionContext>>(node)) {
    return BooleanNodeType::NotEqual;
  }
  if(node_cast<LogicalAndNode<ExecutionContext>>(node)) {
    return BooleanNodeType::And;
  }
  if(node_cast<LogicalOrNode<ExecutionContext>>(node)) {
    return BooleanNodeType::Or;
  }
  if(node_cast<LogicalXorNode<ExecutionContext>>(node)) {
    return BooleanNodeType::Xor;
  }
  if(node_cast<LogicalNotNode<ExecutionContext>>(node)) {
    return BooleanNodeType::Not;
  }
  return BooleanNodeType::Always;
}

auto scalar_node_type(const ExecutionNode& node) noexcept -> ScalarNodeType
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
  return ScalarNodeType::Value;
}

auto make_boolean_node(BooleanNodeType type) -> ExecutionNode
{
  const auto scalar = [] { return ExecutionNode{ValueNode{0.0}}; };
  const auto condition = [] { return ExecutionNode{TrueNode{}}; };
  switch(type) {
  case BooleanNodeType::Always:
    return ExecutionNode{TrueNode{}};
  case BooleanNodeType::Never:
    return ExecutionNode{FalseNode{}};
  case BooleanNodeType::GreaterThan:
    return ExecutionNode{GreaterThanNode<ExecutionContext>{scalar(), scalar()}};
  case BooleanNodeType::GreaterEqual:
    return ExecutionNode{
     GreaterEqualNode<ExecutionContext>{scalar(), scalar()}};
  case BooleanNodeType::LessThan:
    return ExecutionNode{LessThanNode<ExecutionContext>{scalar(), scalar()}};
  case BooleanNodeType::LessEqual:
    return ExecutionNode{LessEqualNode<ExecutionContext>{scalar(), scalar()}};
  case BooleanNodeType::Equal:
    return ExecutionNode{EqualNode<ExecutionContext>{scalar(), scalar()}};
  case BooleanNodeType::NotEqual:
    return ExecutionNode{NotEqualNode<ExecutionContext>{scalar(), scalar()}};
  case BooleanNodeType::And:
    return ExecutionNode{
     LogicalAndNode<ExecutionContext>{condition(), condition()}};
  case BooleanNodeType::Or:
    return ExecutionNode{
     LogicalOrNode<ExecutionContext>{condition(), condition()}};
  case BooleanNodeType::Xor:
    return ExecutionNode{
     LogicalXorNode<ExecutionContext>{condition(), condition()}};
  case BooleanNodeType::Not:
    return ExecutionNode{LogicalNotNode<ExecutionContext>{condition()}};
  }
  return ExecutionNode{TrueNode{}};
}

auto make_scalar_node(ScalarNodeType type) -> ExecutionNode
{
  switch(type) {
  case ScalarNodeType::Value:
    return ExecutionNode{ValueNode{0.0}};
  case ScalarNodeType::Equity:
    return ExecutionNode{EquityNode{}};
  case ScalarNodeType::EquityPercent:
    return ExecutionNode{EquityPercentNode{}};
  case ScalarNodeType::Drawdown:
    return ExecutionNode{DrawdownNode{}};
  case ScalarNodeType::StrategyPerformance:
    return ExecutionNode{
     StrategyPerformanceNode{StrategyPerformanceMetric::LifetimeCount}};
  }
  return ExecutionNode{ValueNode{0.0}};
}

auto render_scalar_node(ExecutionNode& node, int& next_id) -> bool;
auto render_boolean_node(ExecutionNode& node, int& next_id) -> bool;

template<typename TComparison>
auto render_comparison(ExecutionNode& node, int& next_id) -> bool
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
    node = ExecutionNode{TComparison{std::move(target), std::move(threshold)}};
  }
  return changed;
}

template<typename TLogical>
auto render_binary_logical(ExecutionNode& node, int& next_id) -> bool
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
    node = ExecutionNode{TLogical{std::move(first), std::move(second)}};
  }
  return changed;
}

auto render_scalar_node(ExecutionNode& node, int& next_id) -> bool
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
      node = ExecutionNode{ValueNode{value}};
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
      node = ExecutionNode{StrategyPerformanceNode{
       static_cast<StrategyPerformanceMetric>(metric_index)}};
      changed = true;
    }
    break;
  }
  case ScalarNodeType::Equity:
  case ScalarNodeType::EquityPercent:
  case ScalarNodeType::Drawdown:
    break;
  }

  ImGui::PopID();
  return changed;
}

auto render_boolean_node(ExecutionNode& node, int& next_id) -> bool
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
    changed |=
     render_comparison<GreaterThanNode<ExecutionContext>>(node, next_id);
    break;
  case BooleanNodeType::GreaterEqual:
    changed |=
     render_comparison<GreaterEqualNode<ExecutionContext>>(node, next_id);
    break;
  case BooleanNodeType::LessThan:
    changed |= render_comparison<LessThanNode<ExecutionContext>>(node, next_id);
    break;
  case BooleanNodeType::LessEqual:
    changed |=
     render_comparison<LessEqualNode<ExecutionContext>>(node, next_id);
    break;
  case BooleanNodeType::Equal:
    changed |= render_comparison<EqualNode<ExecutionContext>>(node, next_id);
    break;
  case BooleanNodeType::NotEqual:
    changed |= render_comparison<NotEqualNode<ExecutionContext>>(node, next_id);
    break;
  case BooleanNodeType::And:
    changed |=
     render_binary_logical<LogicalAndNode<ExecutionContext>>(node, next_id);
    break;
  case BooleanNodeType::Or:
    changed |=
     render_binary_logical<LogicalOrNode<ExecutionContext>>(node, next_id);
    break;
  case BooleanNodeType::Xor:
    changed |=
     render_binary_logical<LogicalXorNode<ExecutionContext>>(node, next_id);
    break;
  case BooleanNodeType::Not: {
    const auto* logical = node_cast<LogicalNotNode<ExecutionContext>>(node);
    auto condition = logical->other_condition();
    ImGui::Indent();
    ImGui::TextUnformatted("Condition");
    if(render_boolean_node(condition, next_id)) {
      node =
       ExecutionNode{LogicalNotNode<ExecutionContext>{std::move(condition)}};
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

export auto
execution_filter_node_editor(ErasedNode<ExecutionFilterMethodContext>& root)
 -> bool
{
  auto next_id = 0;
  return render_boolean_node(root, next_id);
}

} // namespace pludux::backtest::gui
