module;

#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:model_inputs;

import pludux;

import :strategy;
import :risk_distance_node;
import :position_node;
import :model;
import :stop_target_price_node;

export namespace pludux::backtest {

void collect_numeric_inputs_from_node(
 const ErasedNode<BacktestMethodContext>& node,
 std::vector<NumericInputNode>& inputs);

void collect_numeric_inputs_from_node(const NumericInputNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  inputs.push_back(node);
}

void collect_numeric_inputs_from_node(const auto&,
                                      std::vector<NumericInputNode>&)
{
}

void collect_numeric_inputs_from_node(
 const ChangeNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
}

void collect_numeric_inputs_from_node(
 const LookbackNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
}

void collect_numeric_inputs_from_node(
 const SelectOutputNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
}

void collect_numeric_inputs_from_node(
 const HighestNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(
 const LowestNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(
 const PercentageNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.base(), inputs);
}

void collect_numeric_inputs_from_node(
 const SlAmountNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(
 const RiskDistanceAmountNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(
 const RiskDistancePercentNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(
 const RiskDistanceAtrNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
  collect_numeric_inputs_from_node(node.multiplier(), inputs);
}

void collect_numeric_inputs_from_node(
 const TpAmountNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(
 const SlPercentNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(
 const TpPercentNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(
 const SlAtrNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
  collect_numeric_inputs_from_node(node.multiplier(), inputs);
}

void collect_numeric_inputs_from_node(
 const TpAtrNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
  collect_numeric_inputs_from_node(node.multiplier(), inputs);
}

void collect_numeric_inputs_from_node(
 const TpRMultipleNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(
 const PositionRMultipleNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
}

void collect_numeric_inputs_from_node(
 const StddevNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(
 const SmaNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(
 const EmaNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(
 const RmaNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(
 const WmaNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(
 const HmaNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const MacdNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.fast_period(), inputs);
  collect_numeric_inputs_from_node(node.slow_period(), inputs);
  collect_numeric_inputs_from_node(node.signal_period(), inputs);
}

void collect_numeric_inputs_from_node(const AtrNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(
 const RocNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(
 const RsiNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const BbNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
  collect_numeric_inputs_from_node(node.stddev(), inputs);
}

void collect_numeric_inputs_from_node(const KcNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
  collect_numeric_inputs_from_node(node.multiplier(), inputs);
  collect_numeric_inputs_from_node(node.band_atr_period(), inputs);
}

void collect_numeric_inputs_from_node(const RvolNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const DonchianChannelNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const StochNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.k_period(), inputs);
  collect_numeric_inputs_from_node(node.k_smooth(), inputs);
  collect_numeric_inputs_from_node(node.d_period(), inputs);
}

void collect_numeric_inputs_from_node(const StochRsiNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.rsi_source(), inputs);
  collect_numeric_inputs_from_node(node.rsi_period(), inputs);
  collect_numeric_inputs_from_node(node.k_period(), inputs);
  collect_numeric_inputs_from_node(node.k_smooth(), inputs);
  collect_numeric_inputs_from_node(node.d_period(), inputs);
}

void collect_numeric_inputs_from_node(
 const AllOfNode<BacktestMethodContext>& node,
 std::vector<NumericInputNode>& inputs)
{
  for(const auto& condition : node.conditions()) {
    collect_numeric_inputs_from_node(condition, inputs);
  }
}

void collect_numeric_inputs_from_node(
 const AnyOfNode<BacktestMethodContext>& node,
 std::vector<NumericInputNode>& inputs)
{
  for(const auto& condition : node.conditions()) {
    collect_numeric_inputs_from_node(condition, inputs);
  }
}

template<typename TComparator, typename TContext>
void collect_numeric_inputs_from_node(
 const ComparisonNode<TComparator, TContext>& node,
 std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.target(), inputs);
  collect_numeric_inputs_from_node(node.threshold(), inputs);
}

template<typename TOperator, typename TContext>
void collect_numeric_inputs_from_node(
 const BinaryLogicalNode<TOperator, TContext>& node,
 std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.first_condition(), inputs);
  collect_numeric_inputs_from_node(node.second_condition(), inputs);
}

template<typename TOperator, typename TContext>
void collect_numeric_inputs_from_node(
 const UnaryLogicalNode<TOperator, TContext>& node,
 std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.other_condition(), inputs);
}

void collect_numeric_inputs_from_node(
 const CrossoverNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.reference(), inputs);
}

void collect_numeric_inputs_from_node(
 const CrossunderNode<BacktestMethodContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.reference(), inputs);
}

template<typename TBinaryFn, typename TContext>
void collect_numeric_inputs_from_node(const BinaryOperatorNode<TBinaryFn, TContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.operand1(), inputs);
  collect_numeric_inputs_from_node(node.operand2(), inputs);
}

template<typename TUnaryFn, typename TContext>
void collect_numeric_inputs_from_node(const UnaryOperatorNode<TUnaryFn, TContext>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.operand(), inputs);
}

template<typename TNode>
auto collect_if_node(const ErasedNode<BacktestMethodContext>& node,
                     std::vector<NumericInputNode>& inputs) -> bool
{
  const auto* typed_node = node_cast<TNode>(node);
  if(!typed_node) {
    return false;
  }

  collect_numeric_inputs_from_node(*typed_node, inputs);
  return true;
}

void collect_numeric_inputs_from_node(
 const ErasedNode<BacktestMethodContext>& node,
 std::vector<NumericInputNode>& inputs)
{
#define PLUDUX_COLLECT_IF_NODE(TNode)        \
  if(collect_if_node<TNode>(node, inputs)) { \
    return;                                  \
  }

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

auto collect_model_inputs(const Model& model)
 -> std::vector<NumericInputNode>
{
  auto inputs = std::vector<NumericInputNode>{};

  for(const auto& [_, series_node] : model.series_nodes()) {
    collect_numeric_inputs_from_node(series_node, inputs);
  }

  const auto collect_from_position =
    [&inputs](const Model::Position& position) {
     collect_numeric_inputs_from_node(position.entry().signal(), inputs);
     for(const auto& exit : position.exits()) {
       if(exit.enabled()) {
         collect_numeric_inputs_from_node(exit.signal(), inputs);
       }
     }
     collect_numeric_inputs_from_node(position.pyramiding().signal(), inputs);
     collect_numeric_inputs_from_node(position.risk_distance(), inputs);
     for(const auto& stop_loss : position.stop_losses()) {
       if(stop_loss.enabled()) {
         collect_numeric_inputs_from_node(stop_loss.stop_price(), inputs);
       }
     }
     for(const auto& take_profit : position.take_profits()) {
       if(take_profit.enabled()) {
         collect_numeric_inputs_from_node(take_profit.target_price(), inputs);
       }
     }
   };

  collect_from_position(model.long_position());
  collect_from_position(model.short_position());

  return inputs;
}

void assign_strategy_model(Strategy& strategy,
                           ModelStoreHandle model_handle,
                           const Model& model)
{
  auto inputs = collect_model_inputs(model);
  strategy.model_handle(std::move(model_handle));
  strategy.inputs(std::move(inputs));
}

} // namespace pludux::backtest
