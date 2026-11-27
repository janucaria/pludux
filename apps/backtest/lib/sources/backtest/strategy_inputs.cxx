module;

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:strategy_inputs;

import pludux;

import :strategy_setup;
import :execution_model;
import :call_node;
import :extern_field_node;
import :risk_distance_node;
import :position_node;
import :strategy;
import :stop_target_price_node;

namespace pludux::backtest {
namespace {

struct StrategyDeclarations {
  std::vector<NumericInputNode> inputs;
  std::vector<std::string> call_fields;
  std::vector<ExternFieldNode> external_fields;
};

void collect_declarations(const ErasedNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations);

void collect_declarations(const NumericInputNode& node,
                          StrategyDeclarations& declarations)
{
  declarations.inputs.push_back(node);
}

void collect_declarations(const CallNode& node,
                          StrategyDeclarations& declarations)
{
  auto& fields = declarations.call_fields;
  if(std::ranges::find(fields, node.field()) == fields.end()) {
    fields.push_back(node.field());
  }
}

void collect_declarations(const auto&, StrategyDeclarations&)
{
}

void collect_declarations(const ExternFieldNode& node,
                          StrategyDeclarations& declarations)
{
  auto& fields = declarations.external_fields;
  if(std::ranges::find(fields, node) == fields.end())
    fields.push_back(node);
}

void collect_declarations(const ChangeNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
}

void collect_declarations(const LookbackNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
}

void collect_declarations(const SelectOutputNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
}

void collect_declarations(const HighestNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const LowestNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const PercentageNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.base(), declarations);
}

void collect_declarations(const SlAmountNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.value(), declarations);
}

void collect_declarations(
 const RiskDistanceAmountNode<BacktestMethodContext>& node,
 StrategyDeclarations& declarations)
{
  collect_declarations(node.value(), declarations);
}

void collect_declarations(
 const RiskDistancePercentNode<BacktestMethodContext>& node,
 StrategyDeclarations& declarations)
{
  collect_declarations(node.value(), declarations);
}

void collect_declarations(
 const RiskDistanceAtrNode<BacktestMethodContext>& node,
 StrategyDeclarations& declarations)
{
  collect_declarations(node.period(), declarations);
  collect_declarations(node.multiplier(), declarations);
}

void collect_declarations(const TpAmountNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.value(), declarations);
}

void collect_declarations(const SlPercentNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.value(), declarations);
}

void collect_declarations(const TpPercentNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.value(), declarations);
}

void collect_declarations(const SlAtrNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.period(), declarations);
  collect_declarations(node.multiplier(), declarations);
}

void collect_declarations(const TpAtrNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.period(), declarations);
  collect_declarations(node.multiplier(), declarations);
}

void collect_declarations(const TpRMultipleNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.value(), declarations);
}

void collect_declarations(
 const PositionRMultipleNode<BacktestMethodContext>& node,
 StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
}

void collect_declarations(const StddevNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const SmaNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const EmaNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const RmaNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const WmaNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const HmaNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const MacdNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.fast_period(), declarations);
  collect_declarations(node.slow_period(), declarations);
  collect_declarations(node.signal_period(), declarations);
}

void collect_declarations(const AtrNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const RocNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const RsiNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const BbNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
  collect_declarations(node.stddev(), declarations);
}

void collect_declarations(const KcNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.period(), declarations);
  collect_declarations(node.multiplier(), declarations);
  collect_declarations(node.band_atr_period(), declarations);
}

void collect_declarations(const RvolNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.period(), declarations);
}

void collect_declarations(
 const DonchianChannelNode<BacktestMethodContext>& node,
 StrategyDeclarations& declarations)
{
  collect_declarations(node.period(), declarations);
}

void collect_declarations(const StochNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.k_period(), declarations);
  collect_declarations(node.k_smooth(), declarations);
  collect_declarations(node.d_period(), declarations);
}

void collect_declarations(const StochRsiNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.rsi_source(), declarations);
  collect_declarations(node.rsi_period(), declarations);
  collect_declarations(node.k_period(), declarations);
  collect_declarations(node.k_smooth(), declarations);
  collect_declarations(node.d_period(), declarations);
}

void collect_declarations(const AllOfNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  for(const auto& condition : node.conditions()) {
    collect_declarations(condition, declarations);
  }
}

void collect_declarations(const AnyOfNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  for(const auto& condition : node.conditions()) {
    collect_declarations(condition, declarations);
  }
}

template<typename TComparator, typename TContext>
void collect_declarations(const ComparisonNode<TComparator, TContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.target(), declarations);
  collect_declarations(node.threshold(), declarations);
}

template<typename TContext, bool ByPercent, ComparisonDirection Direction>
void collect_declarations(
 const OffsetComparisonNode<TContext, ByPercent, Direction>& node,
 StrategyDeclarations& declarations)
{
  collect_declarations(node.target(), declarations);
  collect_declarations(node.reference(), declarations);
  collect_declarations(node.amount(), declarations);
}

template<typename TOperator, typename TContext>
void collect_declarations(const BinaryLogicalNode<TOperator, TContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.first_condition(), declarations);
  collect_declarations(node.second_condition(), declarations);
}

template<typename TOperator, typename TContext>
void collect_declarations(const UnaryLogicalNode<TOperator, TContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.other_condition(), declarations);
}

void collect_declarations(const CrossoverNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.reference(), declarations);
}

void collect_declarations(const CrossunderNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.source(), declarations);
  collect_declarations(node.reference(), declarations);
}

template<typename TBinaryFn, typename TContext>
void collect_declarations(const BinaryOperatorNode<TBinaryFn, TContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.operand1(), declarations);
  collect_declarations(node.operand2(), declarations);
}

template<typename TUnaryFn, typename TContext>
void collect_declarations(const UnaryOperatorNode<TUnaryFn, TContext>& node,
                          StrategyDeclarations& declarations)
{
  collect_declarations(node.operand(), declarations);
}

template<typename TNode>
auto collect_if_node(const ErasedNode<BacktestMethodContext>& node,
                     StrategyDeclarations& declarations) -> bool
{
  const auto* typed_node = node_cast<TNode>(node);
  if(!typed_node) {
    return false;
  }

  collect_declarations(*typed_node, declarations);
  return true;
}

void collect_declarations(const ErasedNode<BacktestMethodContext>& node,
                          StrategyDeclarations& declarations)
{
#define PLUDUX_COLLECT_IF_NODE(TNode)              \
  if(collect_if_node<TNode>(node, declarations)) { \
    return;                                        \
  }

  PLUDUX_COLLECT_IF_NODE(CallNode)
  PLUDUX_COLLECT_IF_NODE(ExternFieldNode)
  PLUDUX_COLLECT_IF_NODE(NumericInputNode)
  PLUDUX_COLLECT_IF_NODE(ValueNode)
  PLUDUX_COLLECT_IF_NODE(DataNode)
  PLUDUX_COLLECT_IF_NODE(OpenNode)
  PLUDUX_COLLECT_IF_NODE(HighNode)
  PLUDUX_COLLECT_IF_NODE(LowNode)
  PLUDUX_COLLECT_IF_NODE(CloseNode)
  PLUDUX_COLLECT_IF_NODE(VolumeNode)
  PLUDUX_COLLECT_IF_NODE(ChangeNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(LookbackNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(SelectOutputNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(SeriesNode)
  PLUDUX_COLLECT_IF_NODE(HighestNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(LowestNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(PercentageNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(StddevNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(SmaNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(EmaNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(RmaNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(WmaNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(HmaNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(MacdNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(TrNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(AtrNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(RocNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(RsiNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(RvolNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(BbNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(KcNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(DonchianChannelNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(StochNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(StochRsiNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(AllOfNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(AnyOfNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(GreaterEqualNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(GreaterThanNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(LessThanNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(LessEqualNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(EqualNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(NotEqualNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(AboveByNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(AboveByPercentNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(BelowByNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(BelowByPercentNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(TrueNode)
  PLUDUX_COLLECT_IF_NODE(FalseNode)
  PLUDUX_COLLECT_IF_NODE(LogicalAndNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(LogicalOrNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(LogicalNotNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(LogicalXorNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(CrossoverNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(CrossunderNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(MultiplyNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(DivideNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(AddNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(SubtractNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(NegateNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(AbsNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(AbsDiffNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(SqrtNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(MaxNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(MinNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(PositivePartNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(NegativePartNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(RiskDistanceAmountNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(RiskDistancePercentNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(RiskDistanceAtrNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(SlAmountNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(TpAmountNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(SlPercentNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(TpPercentNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(SlAtrNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(TpAtrNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(TpRMultipleNode<BacktestMethodContext>)
  PLUDUX_COLLECT_IF_NODE(PositionRMultipleNode<BacktestMethodContext>)

#undef PLUDUX_COLLECT_IF_NODE
}

auto collect_declarations(const Strategy& strategy) -> StrategyDeclarations
{
  auto declarations = StrategyDeclarations{};

  for(const auto& [_, series_node] : strategy.series_nodes()) {
    collect_declarations(series_node, declarations);
  }

  const auto collect_from_position =
   [&declarations](const Strategy::Position& position) {
     if(position.entry().entry_source() == EntrySource::Signal) {
       collect_declarations(position.entry().signal(), declarations);
       collect_declarations(position.entry().price(), declarations);
     }
     for(const auto& exit : position.exits()) {
       if(exit.enabled()) {
         collect_declarations(exit.signal(), declarations);
         collect_declarations(exit.price(), declarations);
       }
     }
     collect_declarations(position.pyramiding().signal(), declarations);
     collect_declarations(position.pyramiding().price(), declarations);
     collect_declarations(position.risk_distance(), declarations);
     for(const auto& stop_loss : position.stop_losses()) {
       if(stop_loss.enabled()) {
         collect_declarations(stop_loss.stop_price(), declarations);
       }
     }
     for(const auto& take_profit : position.take_profits()) {
       if(take_profit.enabled()) {
         collect_declarations(take_profit.target_price(), declarations);
       }
     }
   };

  collect_from_position(strategy.long_position());
  collect_from_position(strategy.short_position());

  return declarations;
}

} // namespace
} // namespace pludux::backtest

export namespace pludux::backtest {

auto collect_strategy_inputs(const Strategy& strategy)
 -> std::vector<NumericInputNode>
{
  return collect_declarations(strategy).inputs;
}

auto collect_strategy_call_fields(const Strategy& strategy)
 -> std::vector<std::string>
{
  return collect_declarations(strategy).call_fields;
}

auto collect_strategy_external_fields(const Strategy& strategy)
 -> std::vector<ExternFieldNode>
{
  return collect_declarations(strategy).external_fields;
}

void sync_strategy_inputs(StrategySetup& setup, const Strategy& strategy)
{
  auto inputs = collect_strategy_inputs(strategy);
  const auto& previous = setup.inputs();
  for(std::size_t index = 0; index < inputs.size() && index < previous.size(); ++index) {
    inputs[index].value(previous[index].value());
  }
  setup.inputs(std::move(inputs));
}

void assign_setup_strategy(StrategySetup& setup,
                           StrategyStoreHandle strategy_handle,
                           const Strategy& strategy)
{
  auto inputs = collect_strategy_inputs(strategy);
  setup.strategy_handle(std::move(strategy_handle));
  setup.inputs(std::move(inputs));
}

} // namespace pludux::backtest
