module;

#include <string>
#include <vector>

export module pludux.backtest:strategy_inputs;

import pludux;

import :risk_distance_node;
import :strategy;
import :stop_target_price_node;

export namespace pludux::backtest {

void collect_numeric_inputs_from_node(const ErasedNode& node,
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

void collect_numeric_inputs_from_node(const ChangeNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
}

void collect_numeric_inputs_from_node(const LookbackNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
}

void collect_numeric_inputs_from_node(const SelectOutputNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
}

void collect_numeric_inputs_from_node(const HighestNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const LowestNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const PercentageNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.base(), inputs);
}

void collect_numeric_inputs_from_node(const SlAmountNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(const RiskDistanceAmountNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(const RiskDistancePercentNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(const RiskDistanceAtrNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
  collect_numeric_inputs_from_node(node.multiplier(), inputs);
}

void collect_numeric_inputs_from_node(const TpAmountNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(const SlPercentNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(const TpPercentNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(const SlAtrNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
  collect_numeric_inputs_from_node(node.multiplier(), inputs);
}

void collect_numeric_inputs_from_node(const TpAtrNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
  collect_numeric_inputs_from_node(node.multiplier(), inputs);
}

void collect_numeric_inputs_from_node(const TpRMultipleNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.value(), inputs);
}

void collect_numeric_inputs_from_node(const StddevNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const SmaNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const EmaNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const RmaNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const WmaNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const HmaNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const MacdNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.fast_period(), inputs);
  collect_numeric_inputs_from_node(node.slow_period(), inputs);
  collect_numeric_inputs_from_node(node.signal_period(), inputs);
}

void collect_numeric_inputs_from_node(const AtrNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const RocNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const RsiNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const BbNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
  collect_numeric_inputs_from_node(node.stddev(), inputs);
}

void collect_numeric_inputs_from_node(const KcNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.period(), inputs);
  collect_numeric_inputs_from_node(node.multiplier(), inputs);
  collect_numeric_inputs_from_node(node.band_atr_period(), inputs);
}

void collect_numeric_inputs_from_node(const RvolNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const DonchianChannelNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.period(), inputs);
}

void collect_numeric_inputs_from_node(const StochNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.k_period(), inputs);
  collect_numeric_inputs_from_node(node.k_smooth(), inputs);
  collect_numeric_inputs_from_node(node.d_period(), inputs);
}

void collect_numeric_inputs_from_node(const StochRsiNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.rsi_source(), inputs);
  collect_numeric_inputs_from_node(node.rsi_period(), inputs);
  collect_numeric_inputs_from_node(node.k_period(), inputs);
  collect_numeric_inputs_from_node(node.k_smooth(), inputs);
  collect_numeric_inputs_from_node(node.d_period(), inputs);
}

void collect_numeric_inputs_from_node(const AllOfNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  for(const auto& condition : node.conditions()) {
    collect_numeric_inputs_from_node(condition, inputs);
  }
}

void collect_numeric_inputs_from_node(const AnyOfNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  for(const auto& condition : node.conditions()) {
    collect_numeric_inputs_from_node(condition, inputs);
  }
}

template<typename TComparator>
void collect_numeric_inputs_from_node(const ComparisonNode<TComparator>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.target(), inputs);
  collect_numeric_inputs_from_node(node.threshold(), inputs);
}

template<typename TOperator>
void collect_numeric_inputs_from_node(const BinaryLogicalNode<TOperator>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.first_condition(), inputs);
  collect_numeric_inputs_from_node(node.second_condition(), inputs);
}

template<typename TOperator>
void collect_numeric_inputs_from_node(const UnaryLogicalNode<TOperator>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.other_condition(), inputs);
}

void collect_numeric_inputs_from_node(const CrossoverNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.reference(), inputs);
}

void collect_numeric_inputs_from_node(const CrossunderNode& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.source(), inputs);
  collect_numeric_inputs_from_node(node.reference(), inputs);
}

template<typename TBinaryFn>
void collect_numeric_inputs_from_node(const BinaryOperatorNode<TBinaryFn>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.operand1(), inputs);
  collect_numeric_inputs_from_node(node.operand2(), inputs);
}

template<typename TUnaryFn>
void collect_numeric_inputs_from_node(const UnaryOperatorNode<TUnaryFn>& node,
                                      std::vector<NumericInputNode>& inputs)
{
  collect_numeric_inputs_from_node(node.operand(), inputs);
}

template<typename TNode>
auto collect_if_node(const ErasedNode& node,
                     std::vector<NumericInputNode>& inputs) -> bool
{
  const auto* typed_node = node_cast<TNode>(node);
  if(!typed_node) {
    return false;
  }

  collect_numeric_inputs_from_node(*typed_node, inputs);
  return true;
}

void collect_numeric_inputs_from_node(const ErasedNode& node,
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
  PLUDUX_COLLECT_IF_NODE(ChangeNode)
  PLUDUX_COLLECT_IF_NODE(LookbackNode)
  PLUDUX_COLLECT_IF_NODE(SelectOutputNode)
  PLUDUX_COLLECT_IF_NODE(SeriesNode)
  PLUDUX_COLLECT_IF_NODE(HighestNode)
  PLUDUX_COLLECT_IF_NODE(LowestNode)
  PLUDUX_COLLECT_IF_NODE(PercentageNode)
  PLUDUX_COLLECT_IF_NODE(StddevNode)
  PLUDUX_COLLECT_IF_NODE(SmaNode)
  PLUDUX_COLLECT_IF_NODE(EmaNode)
  PLUDUX_COLLECT_IF_NODE(RmaNode)
  PLUDUX_COLLECT_IF_NODE(WmaNode)
  PLUDUX_COLLECT_IF_NODE(HmaNode)
  PLUDUX_COLLECT_IF_NODE(MacdNode)
  PLUDUX_COLLECT_IF_NODE(TrNode)
  PLUDUX_COLLECT_IF_NODE(AtrNode)
  PLUDUX_COLLECT_IF_NODE(RocNode)
  PLUDUX_COLLECT_IF_NODE(RsiNode)
  PLUDUX_COLLECT_IF_NODE(RvolNode)
  PLUDUX_COLLECT_IF_NODE(BbNode)
  PLUDUX_COLLECT_IF_NODE(KcNode)
  PLUDUX_COLLECT_IF_NODE(DonchianChannelNode)
  PLUDUX_COLLECT_IF_NODE(StochNode)
  PLUDUX_COLLECT_IF_NODE(StochRsiNode)
  PLUDUX_COLLECT_IF_NODE(AllOfNode)
  PLUDUX_COLLECT_IF_NODE(AnyOfNode)
  PLUDUX_COLLECT_IF_NODE(GreaterEqualNode)
  PLUDUX_COLLECT_IF_NODE(GreaterThanNode)
  PLUDUX_COLLECT_IF_NODE(LessThanNode)
  PLUDUX_COLLECT_IF_NODE(LessEqualNode)
  PLUDUX_COLLECT_IF_NODE(EqualNode)
  PLUDUX_COLLECT_IF_NODE(NotEqualNode)
  PLUDUX_COLLECT_IF_NODE(TrueNode)
  PLUDUX_COLLECT_IF_NODE(FalseNode)
  PLUDUX_COLLECT_IF_NODE(LogicalAndNode)
  PLUDUX_COLLECT_IF_NODE(LogicalOrNode)
  PLUDUX_COLLECT_IF_NODE(LogicalNotNode)
  PLUDUX_COLLECT_IF_NODE(LogicalXorNode)
  PLUDUX_COLLECT_IF_NODE(CrossoverNode)
  PLUDUX_COLLECT_IF_NODE(CrossunderNode)
  PLUDUX_COLLECT_IF_NODE(MultiplyNode)
  PLUDUX_COLLECT_IF_NODE(DivideNode)
  PLUDUX_COLLECT_IF_NODE(AddNode)
  PLUDUX_COLLECT_IF_NODE(SubtractNode)
  PLUDUX_COLLECT_IF_NODE(NegateNode)
  PLUDUX_COLLECT_IF_NODE(AbsNode)
  PLUDUX_COLLECT_IF_NODE(AbsDiffNode)
  PLUDUX_COLLECT_IF_NODE(SqrtNode)
  PLUDUX_COLLECT_IF_NODE(MaxNode)
  PLUDUX_COLLECT_IF_NODE(MinNode)
  PLUDUX_COLLECT_IF_NODE(PositivePartNode)
  PLUDUX_COLLECT_IF_NODE(NegativePartNode)
  PLUDUX_COLLECT_IF_NODE(RiskDistanceAmountNode)
  PLUDUX_COLLECT_IF_NODE(RiskDistancePercentNode)
  PLUDUX_COLLECT_IF_NODE(RiskDistanceAtrNode)
  PLUDUX_COLLECT_IF_NODE(SlAmountNode)
  PLUDUX_COLLECT_IF_NODE(TpAmountNode)
  PLUDUX_COLLECT_IF_NODE(SlPercentNode)
  PLUDUX_COLLECT_IF_NODE(TpPercentNode)
  PLUDUX_COLLECT_IF_NODE(SlAtrNode)
  PLUDUX_COLLECT_IF_NODE(TpAtrNode)
  PLUDUX_COLLECT_IF_NODE(TpRMultipleNode)

#undef PLUDUX_COLLECT_IF_NODE
}

auto collect_numeric_inputs(const Strategy& strategy)
 -> std::vector<NumericInputNode>
{
  auto inputs = std::vector<NumericInputNode>{};

  for(const auto& [_, series_node] : strategy.series_nodes()) {
    collect_numeric_inputs_from_node(series_node, inputs);
  }

  const auto collect_from_position = [&inputs](
                                      const Strategy::Position& position) {
    collect_numeric_inputs_from_node(position.entry().signal(), inputs);
    collect_numeric_inputs_from_node(position.entry().price(), inputs);
    for(const auto& exit : position.exits()) {
      if(exit.enabled()) {
        collect_numeric_inputs_from_node(exit.signal(), inputs);
        collect_numeric_inputs_from_node(exit.price(), inputs);
      }
    }
    collect_numeric_inputs_from_node(position.pyramiding().signal(), inputs);
    collect_numeric_inputs_from_node(position.pyramiding().price(), inputs);
    collect_numeric_inputs_from_node(position.risk_distance(), inputs);
    collect_numeric_inputs_from_node(position.stop_loss().stop_price(), inputs);
    for(const auto& take_profit : position.take_profits()) {
      if(take_profit.enabled()) {
        collect_numeric_inputs_from_node(take_profit.target_price(), inputs);
      }
    }
  };

  collect_from_position(strategy.long_position());
  collect_from_position(strategy.short_position());

  return inputs;
}

} // namespace pludux::backtest
