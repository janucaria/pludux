module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

export module pludux:node_to_erased_method;

import :methods;
import :nodes;

export namespace pludux {

class NodeToErasedMethodContext {
public:
  NodeToErasedMethodContext() = default;

  explicit NodeToErasedMethodContext(
   const std::vector<NumericInputNode>& inputs)
  : inputs_{&inputs}
  {
  }

  auto consume(this NodeToErasedMethodContext& self,
               const NumericInputNode& fallback) noexcept
   -> const NumericInputNode&
  {
    if(self.inputs_ && self.input_index_ < self.inputs_->size()) {
      return self.inputs_->at(self.input_index_++);
    }
    ++self.input_index_;
    return fallback;
  }

  auto input_index(this const NodeToErasedMethodContext& self) noexcept
   -> std::size_t
  {
    return self.input_index_;
  }

private:
  const std::vector<NumericInputNode>* inputs_{nullptr};
  std::size_t input_index_{0};
};

auto node_to_erased_method(const ErasedNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod;

auto resolve_numeric_input_value(const NumericInputNode& node) noexcept
 -> double
{
  switch(node.representation()) {
  case NumericInputNode::ValueRepresentation::Decimal:
    return node.value();
  case NumericInputNode::ValueRepresentation::SignedInteger:
    return std::trunc(node.value());
  case NumericInputNode::ValueRepresentation::UnsignedInteger:
    return std::max(0.0, std::trunc(node.value()));
  default:
    return node.value();
  }
}

auto node_to_erased_method(const NumericInputNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  const auto& input = context.consume(node);
  return AnySeriesMethod{ValueMethod{resolve_numeric_input_value(input)}};
}

auto node_to_erased_method(const ValueNode& node, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{ValueMethod{node.value()}};
}

auto node_to_erased_method(const DataNode& node, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{DataMethod{node.field()}};
}

auto node_to_erased_method(const OpenNode&, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{OpenMethod{}};
}

auto node_to_erased_method(const HighNode&, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{HighMethod{}};
}

auto node_to_erased_method(const LowNode&, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{LowMethod{}};
}

auto node_to_erased_method(const CloseNode&, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{CloseMethod{}};
}

auto node_to_erased_method(const VolumeNode&, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{VolumeMethod{}};
}

auto node_to_erased_method(const ChangeNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   ChangeMethod{node_to_erased_method(node.source(), context)}};
}

auto node_to_erased_method(const LookbackNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{LookbackMethod{
   node_to_erased_method(node.source(), context), node.period()}};
}

auto node_to_erased_method(const SelectOutputNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   SelectOutputMethod{node_to_erased_method(node.source(), context),
                      static_cast<MethodOutput>(node.output())}};
}

auto node_to_erased_method(const SeriesNode& node, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{SeriesNodeMethod{node.name()}};
}

auto node_to_erased_method(const SeriesValueNode& node,
                           NodeToErasedMethodContext&) -> AnySeriesMethod
{
  return AnySeriesMethod{SeriesValueMethod{node.name()}};
}

auto node_to_erased_method(const AdaptiveMaNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   AdaptiveMaMethod{node_to_erased_method(node.source(), context),
                    static_cast<MaMethodType>(node.ma_type()),
                    node.period()}};
}

auto node_to_erased_method(const HighestNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   HighestMethod{node_to_erased_method(node.source(), context), node.period()}};
}

auto node_to_erased_method(const LowestNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   LowestMethod{node_to_erased_method(node.source(), context), node.period()}};
}

auto node_to_erased_method(const PercentageNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{PercentageMethod{
   node_to_erased_method(node.base(), context), node.percent()}};
}

auto node_to_erased_method(const StddevNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   StddevMethod{node_to_erased_method(node.source(), context), node.period()}};
}

auto node_to_erased_method(const SmaNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   SmaMethod{node_to_erased_method(node.source(), context), node.period()}};
}

auto node_to_erased_method(const EmaNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   EmaMethod{node_to_erased_method(node.source(), context), node.period()}};
}

auto node_to_erased_method(const RmaNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   RmaMethod{node_to_erased_method(node.source(), context), node.period()}};
}

auto node_to_erased_method(const WmaNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   WmaMethod{node_to_erased_method(node.source(), context), node.period()}};
}

auto node_to_erased_method(const HmaNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   HmaMethod{node_to_erased_method(node.source(), context), node.period()}};
}

auto node_to_erased_method(const MacdNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   MacdMethod{node_to_erased_method(node.source(), context),
              node.short_period(),
              node.long_period(),
              node.signal_period()}};
}

auto node_to_erased_method(const TrNode&, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{TrMethod{}};
}

auto node_to_erased_method(const AtrNode& node, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{AtrMethod{
   node.period(), static_cast<MaMethodType>(node.ma_smoothing_type())}};
}

auto node_to_erased_method(const RocNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   RocMethod{node_to_erased_method(node.source(), context), node.period()}};
}

auto node_to_erased_method(const RsiNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   RsiMethod{node_to_erased_method(node.source(), context), node.period()}};
}

auto node_to_erased_method(const RvolNode& node, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{RvolMethod{node.period()}};
}

auto node_to_erased_method(const BbNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   BbMethod{node_to_erased_method(node.source(), context),
            node.period(),
            node.stddev(),
            static_cast<MaMethodType>(node.ma_node_type())}};
}

auto node_to_erased_method(const KcNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   KcMethod{node_to_erased_method(node.source(), context),
            node.period(),
            node.multiplier(),
            node.band_atr_period(),
            static_cast<KcBandMethodType>(node.band_node_type()),
            static_cast<MaMethodType>(node.ma_node_type())}};
}

auto node_to_erased_method(const DonchianChannelNode& node,
                           NodeToErasedMethodContext&) -> AnySeriesMethod
{
  return AnySeriesMethod{DonchianChannelMethod{node.period()}};
}

auto node_to_erased_method(const StochNode& node, NodeToErasedMethodContext&)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   StochMethod{node.k_period(), node.k_smooth(), node.d_period()}};
}

auto node_to_erased_method(const StochRsiNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   StochRsiMethod{node_to_erased_method(node.rsi_source(), context),
                  node.rsi_period(),
                  node.k_period(),
                  node.k_smooth(),
                  node.d_period()}};
}

auto node_to_erased_method(const AllOfNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  auto conditions = std::vector<AnySeriesMethod>{};
  conditions.reserve(node.conditions().size());
  for(const auto& condition : node.conditions()) {
    conditions.emplace_back(node_to_erased_method(condition, context));
  }

  return AnySeriesMethod{AllOfMethod<AnySeriesMethod>{std::move(conditions)}};
}

auto node_to_erased_method(const AnyOfNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  auto conditions = std::vector<AnySeriesMethod>{};
  conditions.reserve(node.conditions().size());
  for(const auto& condition : node.conditions()) {
    conditions.emplace_back(node_to_erased_method(condition, context));
  }

  return AnySeriesMethod{AnyOfMethod<AnySeriesMethod>{std::move(conditions)}};
}

template<typename TComparator>
auto node_to_erased_method(const ComparisonNode<TComparator>& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  auto target = node_to_erased_method(node.target(), context);
  auto threshold = node_to_erased_method(node.threshold(), context);
  return AnySeriesMethod{
   ComparisonMethod<TComparator, AnySeriesMethod, AnySeriesMethod>{
    std::move(target), std::move(threshold)}};
}

template<bool boolean_value>
auto node_to_erased_method(const BooleanNode<boolean_value>&,
                           NodeToErasedMethodContext&) -> AnySeriesMethod
{
  return AnySeriesMethod{BooleanMethod<boolean_value>{}};
}

template<typename TOperator>
auto node_to_erased_method(const BinaryLogicalNode<TOperator>& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  auto first_condition = node_to_erased_method(node.first_condition(), context);
  auto second_condition =
   node_to_erased_method(node.second_condition(), context);
  return AnySeriesMethod{
   BinaryLogicalMethod<TOperator, AnySeriesMethod, AnySeriesMethod>{
    std::move(first_condition), std::move(second_condition)}};
}

template<typename TOperator>
auto node_to_erased_method(const UnaryLogicalNode<TOperator>& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{UnaryLogicalMethod<TOperator, AnySeriesMethod>{
   node_to_erased_method(node.other_condition(), context)}};
}

auto node_to_erased_method(const CrossoverNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  auto source = node_to_erased_method(node.source(), context);
  auto reference = node_to_erased_method(node.reference(), context);
  return AnySeriesMethod{CrossoverMethod<AnySeriesMethod, AnySeriesMethod>{
   std::move(source), std::move(reference)}};
}

auto node_to_erased_method(const CrossunderNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  auto source = node_to_erased_method(node.source(), context);
  auto reference = node_to_erased_method(node.reference(), context);
  return AnySeriesMethod{CrossunderMethod<AnySeriesMethod, AnySeriesMethod>{
   std::move(source), std::move(reference)}};
}

template<typename TBinaryFn>
auto node_to_erased_method(const BinaryOperatorNode<TBinaryFn>& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  auto operand1 = node_to_erased_method(node.operand1(), context);
  auto operand2 = node_to_erased_method(node.operand2(), context);
  return AnySeriesMethod{
   BinaryOperatorMethod<TBinaryFn, AnySeriesMethod, AnySeriesMethod>{
    std::move(operand1), std::move(operand2)}};
}

template<typename TUnaryFn>
auto node_to_erased_method(const UnaryOperatorNode<TUnaryFn>& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
  return AnySeriesMethod{UnaryOperatorMethod<TUnaryFn, AnySeriesMethod>{
   node_to_erased_method(node.operand(), context)}};
}

auto node_to_erased_method(const ErasedNode& node,
                           NodeToErasedMethodContext& context)
 -> AnySeriesMethod
{
#define PLUDUX_TRY_NODE_TO_METHOD(TNode)                \
  if(const auto typed_node = node_cast<TNode>(node)) {  \
    return node_to_erased_method(*typed_node, context); \
  }

  PLUDUX_TRY_NODE_TO_METHOD(NumericInputNode)
  PLUDUX_TRY_NODE_TO_METHOD(ValueNode)
  PLUDUX_TRY_NODE_TO_METHOD(DataNode)
  PLUDUX_TRY_NODE_TO_METHOD(OpenNode)
  PLUDUX_TRY_NODE_TO_METHOD(HighNode)
  PLUDUX_TRY_NODE_TO_METHOD(LowNode)
  PLUDUX_TRY_NODE_TO_METHOD(CloseNode)
  PLUDUX_TRY_NODE_TO_METHOD(VolumeNode)
  PLUDUX_TRY_NODE_TO_METHOD(ChangeNode)
  PLUDUX_TRY_NODE_TO_METHOD(LookbackNode)
  PLUDUX_TRY_NODE_TO_METHOD(SelectOutputNode)
  PLUDUX_TRY_NODE_TO_METHOD(SeriesNode)
  PLUDUX_TRY_NODE_TO_METHOD(SeriesValueNode)
  PLUDUX_TRY_NODE_TO_METHOD(AdaptiveMaNode)
  PLUDUX_TRY_NODE_TO_METHOD(HighestNode)
  PLUDUX_TRY_NODE_TO_METHOD(LowestNode)
  PLUDUX_TRY_NODE_TO_METHOD(PercentageNode)
  PLUDUX_TRY_NODE_TO_METHOD(StddevNode)
  PLUDUX_TRY_NODE_TO_METHOD(SmaNode)
  PLUDUX_TRY_NODE_TO_METHOD(EmaNode)
  PLUDUX_TRY_NODE_TO_METHOD(RmaNode)
  PLUDUX_TRY_NODE_TO_METHOD(WmaNode)
  PLUDUX_TRY_NODE_TO_METHOD(HmaNode)
  PLUDUX_TRY_NODE_TO_METHOD(MacdNode)
  PLUDUX_TRY_NODE_TO_METHOD(TrNode)
  PLUDUX_TRY_NODE_TO_METHOD(AtrNode)
  PLUDUX_TRY_NODE_TO_METHOD(RocNode)
  PLUDUX_TRY_NODE_TO_METHOD(RsiNode)
  PLUDUX_TRY_NODE_TO_METHOD(RvolNode)
  PLUDUX_TRY_NODE_TO_METHOD(BbNode)
  PLUDUX_TRY_NODE_TO_METHOD(KcNode)
  PLUDUX_TRY_NODE_TO_METHOD(DonchianChannelNode)
  PLUDUX_TRY_NODE_TO_METHOD(StochNode)
  PLUDUX_TRY_NODE_TO_METHOD(StochRsiNode)
  PLUDUX_TRY_NODE_TO_METHOD(AllOfNode)
  PLUDUX_TRY_NODE_TO_METHOD(AnyOfNode)
  PLUDUX_TRY_NODE_TO_METHOD(GreaterEqualNode)
  PLUDUX_TRY_NODE_TO_METHOD(GreaterThanNode)
  PLUDUX_TRY_NODE_TO_METHOD(LessThanNode)
  PLUDUX_TRY_NODE_TO_METHOD(LessEqualNode)
  PLUDUX_TRY_NODE_TO_METHOD(EqualNode)
  PLUDUX_TRY_NODE_TO_METHOD(NotEqualNode)
  PLUDUX_TRY_NODE_TO_METHOD(TrueNode)
  PLUDUX_TRY_NODE_TO_METHOD(FalseNode)
  PLUDUX_TRY_NODE_TO_METHOD(LogicalAndNode)
  PLUDUX_TRY_NODE_TO_METHOD(LogicalOrNode)
  PLUDUX_TRY_NODE_TO_METHOD(LogicalNotNode)
  PLUDUX_TRY_NODE_TO_METHOD(LogicalXorNode)
  PLUDUX_TRY_NODE_TO_METHOD(CrossoverNode)
  PLUDUX_TRY_NODE_TO_METHOD(CrossunderNode)
  PLUDUX_TRY_NODE_TO_METHOD(MultiplyNode)
  PLUDUX_TRY_NODE_TO_METHOD(DivideNode)
  PLUDUX_TRY_NODE_TO_METHOD(AddNode)
  PLUDUX_TRY_NODE_TO_METHOD(SubtractNode)
  PLUDUX_TRY_NODE_TO_METHOD(NegateNode)
  PLUDUX_TRY_NODE_TO_METHOD(AbsNode)
  PLUDUX_TRY_NODE_TO_METHOD(AbsDiffNode)
  PLUDUX_TRY_NODE_TO_METHOD(SqrtNode)
  PLUDUX_TRY_NODE_TO_METHOD(MaxNode)
  PLUDUX_TRY_NODE_TO_METHOD(MinNode)
  PLUDUX_TRY_NODE_TO_METHOD(PositivePartNode)
  PLUDUX_TRY_NODE_TO_METHOD(NegativePartNode)

#undef PLUDUX_TRY_NODE_TO_METHOD

  throw std::invalid_argument{"Unsupported ErasedNode contained node type"};
}

} // namespace pludux
