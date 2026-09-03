module;

#include <utility>

export module pludux.backtest:risk_distance_node;

import pludux;

import :risk_distance_method;

export namespace pludux::backtest {

template<typename TContext>
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

  explicit RiskDistanceValueNode(ErasedNode<TContext> value)
  : value_{std::move(value)}
  {
  }

  auto operator==(const RiskDistanceValueNode&) const noexcept
   -> bool = default;

  auto value(this const RiskDistanceValueNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.value_;
  }

  void value(this RiskDistanceValueNode& self,
             ErasedNode<TContext> value) noexcept
  {
    self.value_ = std::move(value);
  }

private:
  ErasedNode<TContext> value_;
};

template<typename TContext>
class RiskDistanceAmountNode
: public RiskDistanceValueNode<TContext> {
public:
  using RiskDistanceValueNode<TContext>::RiskDistanceValueNode;

  auto operator==(const RiskDistanceAmountNode&) const noexcept
   -> bool = default;
};

template<typename TContext>
class RiskDistancePercentNode
: public RiskDistanceValueNode<TContext> {
public:
  using RiskDistanceValueNode<TContext>::RiskDistanceValueNode;

  auto operator==(const RiskDistancePercentNode&) const noexcept
   -> bool = default;
};

template<typename TContext>
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

  RiskDistanceAtrNode(ErasedNode<TContext> period,
                            ErasedNode<TContext> multiplier,
                            MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : period_{std::move(period)}
  , multiplier_{std::move(multiplier)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const RiskDistanceAtrNode&) const noexcept -> bool =
   default;

  auto period(this const RiskDistanceAtrNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this RiskDistanceAtrNode& self,
              ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  auto multiplier(this const RiskDistanceAtrNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.multiplier_;
  }

  void multiplier(this RiskDistanceAtrNode& self,
                  ErasedNode<TContext> multiplier) noexcept
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
  ErasedNode<TContext> period_;
  ErasedNode<TContext> multiplier_;
  MaNodeType ma_smoothing_type_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const RiskDistanceAmountNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{RiskDistanceAmountMethod{
   node_to_erased_method<TContext>(node.value(), context)}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const RiskDistancePercentNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{
    RiskDistancePercentMethod{
     node_to_erased_method<TContext>(node.value(), context)}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const RiskDistanceAtrNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{RiskDistanceAtrMethod{
    node_to_erased_method<TContext>(node.period(), context),
    node_to_erased_method<TContext>(node.multiplier(), context),
   static_cast<MaMethodType>(node.ma_smoothing_type())}};
}

} // namespace pludux::backtest
