module;

#include <utility>

export module pludux.backtest:risk_distance_node;

import pludux;

import :risk_distance_method;

export namespace pludux::backtest {

template<typename TNode>
class RiskDistanceValueNode {
public:
  RiskDistanceValueNode()
  : RiskDistanceValueNode{1.0}
  {
  }

  explicit RiskDistanceValueNode(double value)
  : RiskDistanceValueNode{ValueNode{value}}
  {
  }

  explicit RiskDistanceValueNode(ErasedNode value)
  : value_{std::move(value)}
  {
  }

  auto operator==(const RiskDistanceValueNode&) const noexcept
   -> bool = default;

  auto value(this const RiskDistanceValueNode& self) noexcept
   -> const ErasedNode&
  {
    return self.value_;
  }

  void value(this RiskDistanceValueNode& self, ErasedNode value) noexcept
  {
    self.value_ = std::move(value);
  }

private:
  ErasedNode value_;
};

class RiskDistanceAmountNode
: public RiskDistanceValueNode<RiskDistanceAmountNode> {
public:
  using RiskDistanceValueNode::RiskDistanceValueNode;

  auto operator==(const RiskDistanceAmountNode&) const noexcept
   -> bool = default;
};

class RiskDistancePercentNode
: public RiskDistanceValueNode<RiskDistancePercentNode> {
public:
  using RiskDistanceValueNode::RiskDistanceValueNode;

  auto operator==(const RiskDistancePercentNode&) const noexcept
   -> bool = default;
};

class RiskDistanceAtrNode {
public:
  RiskDistanceAtrNode()
  : RiskDistanceAtrNode{14.0, 2.0}
  {
  }

  RiskDistanceAtrNode(double period,
                      double multiplier,
                      MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : RiskDistanceAtrNode{
     ValueNode{period}, ValueNode{multiplier}, ma_smoothing_type}
  {
  }

  RiskDistanceAtrNode(ErasedNode period,
                      ErasedNode multiplier,
                      MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : period_{std::move(period)}
  , multiplier_{std::move(multiplier)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const RiskDistanceAtrNode&) const noexcept -> bool = default;

  auto period(this const RiskDistanceAtrNode& self) noexcept
   -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this RiskDistanceAtrNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  auto multiplier(this const RiskDistanceAtrNode& self) noexcept
   -> const ErasedNode&
  {
    return self.multiplier_;
  }

  void multiplier(this RiskDistanceAtrNode& self,
                  ErasedNode multiplier) noexcept
  {
    self.multiplier_ = std::move(multiplier);
  }

  auto ma_smoothing_type(this const RiskDistanceAtrNode& self) noexcept
   -> MaNodeType
  {
    return self.ma_smoothing_type_;
  }

  void ma_smoothing_type(this RiskDistanceAtrNode& self,
                         MaNodeType type) noexcept
  {
    self.ma_smoothing_type_ = type;
  }

private:
  ErasedNode period_;
  ErasedNode multiplier_;
  MaNodeType ma_smoothing_type_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const RiskDistanceAmountNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   RiskDistanceAmountMethod{node_to_erased_method(node.value(), context)}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const RiskDistancePercentNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   RiskDistancePercentMethod{node_to_erased_method(node.value(), context)}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const RiskDistanceAtrNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   RiskDistanceAtrMethod{node_to_erased_method(node.period(), context),
                         node_to_erased_method(node.multiplier(), context),
                         static_cast<MaMethodType>(node.ma_smoothing_type())}};
}

} // namespace pludux::backtest
