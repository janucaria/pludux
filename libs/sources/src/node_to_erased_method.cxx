module;

#include <stdexcept>
#include <utility>
#include <vector>

export module pludux:node_to_erased_method;

import :methods;
import :nodes;

export namespace pludux {

auto node_to_erased_method(const ErasedNode& node) -> AnySeriesMethod;

auto node_to_erased_method(const InputNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{InputMethod{node.name()}};
}

auto node_to_erased_method(const ValueNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{ValueMethod{node.value()}};
}

auto node_to_erased_method(const DataNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{DataMethod{node.field()}};
}

auto node_to_erased_method(const OpenNode&) -> AnySeriesMethod
{
  return AnySeriesMethod{OpenMethod{}};
}

auto node_to_erased_method(const HighNode&) -> AnySeriesMethod
{
  return AnySeriesMethod{HighMethod{}};
}

auto node_to_erased_method(const LowNode&) -> AnySeriesMethod
{
  return AnySeriesMethod{LowMethod{}};
}

auto node_to_erased_method(const CloseNode&) -> AnySeriesMethod
{
  return AnySeriesMethod{CloseMethod{}};
}

auto node_to_erased_method(const VolumeNode&) -> AnySeriesMethod
{
  return AnySeriesMethod{VolumeMethod{}};
}

auto node_to_erased_method(const ChangeNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{ChangeMethod{node_to_erased_method(node.source())}};
}

auto node_to_erased_method(const LookbackNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   LookbackMethod{node_to_erased_method(node.source()), node.period()}};
}

auto node_to_erased_method(const SelectOutputNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   SelectOutputMethod{node_to_erased_method(node.source()),
                      static_cast<MethodOutput>(node.output())}};
}

auto node_to_erased_method(const SeriesNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{SeriesNodeMethod{node.name()}};
}

auto node_to_erased_method(const SeriesValueNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{SeriesValueMethod{node.name()}};
}

auto node_to_erased_method(const AdaptiveMaNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   AdaptiveMaMethod{node_to_erased_method(node.source()),
                    static_cast<MaMethodType>(node.ma_type()),
                    node.period()}};
}

auto node_to_erased_method(const HighestNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   HighestMethod{node_to_erased_method(node.source()), node.period()}};
}

auto node_to_erased_method(const LowestNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   LowestMethod{node_to_erased_method(node.source()), node.period()}};
}

auto node_to_erased_method(const PercentageNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   PercentageMethod{node_to_erased_method(node.base()), node.percent()}};
}

auto node_to_erased_method(const StddevNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   StddevMethod{node_to_erased_method(node.source()), node.period()}};
}

auto node_to_erased_method(const SmaNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   SmaMethod{node_to_erased_method(node.source()), node.period()}};
}

auto node_to_erased_method(const EmaNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   EmaMethod{node_to_erased_method(node.source()), node.period()}};
}

auto node_to_erased_method(const RmaNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   RmaMethod{node_to_erased_method(node.source()), node.period()}};
}

auto node_to_erased_method(const WmaNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   WmaMethod{node_to_erased_method(node.source()), node.period()}};
}

auto node_to_erased_method(const HmaNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   HmaMethod{node_to_erased_method(node.source()), node.period()}};
}

auto node_to_erased_method(const MacdNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{MacdMethod{node_to_erased_method(node.source()),
                                    node.short_period(),
                                    node.long_period(),
                                    node.signal_period()}};
}

auto node_to_erased_method(const TrNode&) -> AnySeriesMethod
{
  return AnySeriesMethod{TrMethod{}};
}

auto node_to_erased_method(const AtrNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{AtrMethod{
   node.period(), static_cast<MaMethodType>(node.ma_smoothing_type())}};
}

auto node_to_erased_method(const RocNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   RocMethod{node_to_erased_method(node.source()), node.period()}};
}

auto node_to_erased_method(const RsiNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   RsiMethod{node_to_erased_method(node.source()), node.period()}};
}

auto node_to_erased_method(const RvolNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{RvolMethod{node.period()}};
}

auto node_to_erased_method(const BbNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   BbMethod{node_to_erased_method(node.source()),
            node.period(),
            node.stddev(),
            static_cast<MaMethodType>(node.ma_node_type())}};
}

auto node_to_erased_method(const KcNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   KcMethod{node_to_erased_method(node.source()),
            node.period(),
            node.multiplier(),
            node.band_atr_period(),
            static_cast<KcBandMethodType>(node.band_node_type()),
            static_cast<MaMethodType>(node.ma_node_type())}};
}

auto node_to_erased_method(const DonchianChannelNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{DonchianChannelMethod{node.period()}};
}

auto node_to_erased_method(const StochNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   StochMethod{node.k_period(), node.k_smooth(), node.d_period()}};
}

auto node_to_erased_method(const StochRsiNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{
   StochRsiMethod{node_to_erased_method(node.rsi_source()),
                  node.rsi_period(),
                  node.k_period(),
                  node.k_smooth(),
                  node.d_period()}};
}

auto node_to_erased_method(const AllOfNode& node) -> AnySeriesMethod
{
  auto conditions = std::vector<AnySeriesMethod>{};
  conditions.reserve(node.conditions().size());
  for(const auto& condition : node.conditions()) {
    conditions.emplace_back(node_to_erased_method(condition));
  }

  return AnySeriesMethod{AllOfMethod<AnySeriesMethod>{std::move(conditions)}};
}

auto node_to_erased_method(const AnyOfNode& node) -> AnySeriesMethod
{
  auto conditions = std::vector<AnySeriesMethod>{};
  conditions.reserve(node.conditions().size());
  for(const auto& condition : node.conditions()) {
    conditions.emplace_back(node_to_erased_method(condition));
  }

  return AnySeriesMethod{AnyOfMethod<AnySeriesMethod>{std::move(conditions)}};
}

template<typename TComparator>
auto node_to_erased_method(const ComparisonNode<TComparator>& node)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   ComparisonMethod<TComparator, AnySeriesMethod, AnySeriesMethod>{
    node_to_erased_method(node.target()),
    node_to_erased_method(node.threshold())}};
}

template<bool boolean_value>
auto node_to_erased_method(const BooleanNode<boolean_value>&) -> AnySeriesMethod
{
  return AnySeriesMethod{BooleanMethod<boolean_value>{}};
}

template<typename TOperator>
auto node_to_erased_method(const BinaryLogicalNode<TOperator>& node)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   BinaryLogicalMethod<TOperator, AnySeriesMethod, AnySeriesMethod>{
    node_to_erased_method(node.first_condition()),
    node_to_erased_method(node.second_condition())}};
}

template<typename TOperator>
auto node_to_erased_method(const UnaryLogicalNode<TOperator>& node)
 -> AnySeriesMethod
{
  return AnySeriesMethod{UnaryLogicalMethod<TOperator, AnySeriesMethod>{
   node_to_erased_method(node.other_condition())}};
}

auto node_to_erased_method(const CrossoverNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{CrossoverMethod<AnySeriesMethod, AnySeriesMethod>{
   node_to_erased_method(node.source()),
   node_to_erased_method(node.reference())}};
}

auto node_to_erased_method(const CrossunderNode& node) -> AnySeriesMethod
{
  return AnySeriesMethod{CrossunderMethod<AnySeriesMethod, AnySeriesMethod>{
   node_to_erased_method(node.source()),
   node_to_erased_method(node.reference())}};
}

template<typename TBinaryFn>
auto node_to_erased_method(const BinaryOperatorNode<TBinaryFn>& node)
 -> AnySeriesMethod
{
  return AnySeriesMethod{
   BinaryOperatorMethod<TBinaryFn, AnySeriesMethod, AnySeriesMethod>{
    node_to_erased_method(node.operand1()),
    node_to_erased_method(node.operand2())}};
}

template<typename TUnaryFn>
auto node_to_erased_method(const UnaryOperatorNode<TUnaryFn>& node)
 -> AnySeriesMethod
{
  return AnySeriesMethod{UnaryOperatorMethod<TUnaryFn, AnySeriesMethod>{
   node_to_erased_method(node.operand())}};
}

auto node_to_erased_method(const ErasedNode& node) -> AnySeriesMethod
{
#define PLUDUX_TRY_NODE_TO_METHOD(TNode)               \
  if(const auto typed_node = node_cast<TNode>(node)) { \
    return node_to_erased_method(*typed_node);         \
  }

  PLUDUX_TRY_NODE_TO_METHOD(InputNode)
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
