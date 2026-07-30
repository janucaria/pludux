module;

#include <utility>

export module pludux.backtest:stop_target_price_node;

import pludux;

import :stop_target_price_method;

export namespace pludux::backtest {

template<typename TNode>
class StopTargetDistanceNode {
public:
  StopTargetDistanceNode()
  : StopTargetDistanceNode{0.0}
  {
  }

  explicit StopTargetDistanceNode(double value)
  : StopTargetDistanceNode{ValueNode{value}}
  {
  }

  explicit StopTargetDistanceNode(ErasedNode value)
  : value_{std::move(value)}
  {
  }

  auto operator==(const StopTargetDistanceNode&) const noexcept
   -> bool = default;

  auto value(this const StopTargetDistanceNode& self) noexcept
   -> const ErasedNode&
  {
    return self.value_;
  }

  void value(this StopTargetDistanceNode& self, ErasedNode value) noexcept
  {
    self.value_ = std::move(value);
  }

private:
  ErasedNode value_;
};

class SlAmountNode : public StopTargetDistanceNode<SlAmountNode> {
public:
  using StopTargetDistanceNode::StopTargetDistanceNode;

  auto operator==(const SlAmountNode&) const noexcept -> bool = default;
};

class TpAmountNode : public StopTargetDistanceNode<TpAmountNode> {
public:
  using StopTargetDistanceNode::StopTargetDistanceNode;

  auto operator==(const TpAmountNode&) const noexcept -> bool = default;
};

class SlPercentNode : public StopTargetDistanceNode<SlPercentNode> {
public:
  using StopTargetDistanceNode::StopTargetDistanceNode;

  auto operator==(const SlPercentNode&) const noexcept -> bool = default;
};

class TpPercentNode : public StopTargetDistanceNode<TpPercentNode> {
public:
  using StopTargetDistanceNode::StopTargetDistanceNode;

  auto operator==(const TpPercentNode&) const noexcept -> bool = default;
};

class Sl1RNode {
public:
  auto operator==(const Sl1RNode&) const noexcept -> bool = default;
};

class TpRMultipleNode : public StopTargetDistanceNode<TpRMultipleNode> {
public:
  TpRMultipleNode()
  : StopTargetDistanceNode{2.0}
  {
  }

  using StopTargetDistanceNode::StopTargetDistanceNode;

  auto operator==(const TpRMultipleNode&) const noexcept -> bool = default;
};

class InitialEntryPriceNode {
public:
  auto operator==(const InitialEntryPriceNode&) const noexcept
   -> bool = default;
};

class LatestEntryPriceNode {
public:
  auto operator==(const LatestEntryPriceNode&) const noexcept -> bool = default;
};

class AveragePriceNode {
public:
  auto operator==(const AveragePriceNode&) const noexcept -> bool = default;
};

class StopTargetRefPriceNode {
public:
  auto operator==(const StopTargetRefPriceNode&) const noexcept
   -> bool = default;
};

class PositionDirectionNode {
public:
  auto operator==(const PositionDirectionNode&) const noexcept
   -> bool = default;
};

class SlAtrNode {
public:
  SlAtrNode()
  : SlAtrNode{14.0, 2.0}
  {
  }

  SlAtrNode(double period,
            double multiplier,
            MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : SlAtrNode{ValueNode{period}, ValueNode{multiplier}, ma_smoothing_type}
  {
  }

  SlAtrNode(ErasedNode period,
            ErasedNode multiplier,
            MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : period_{std::move(period)}
  , multiplier_{std::move(multiplier)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const SlAtrNode&) const noexcept -> bool = default;

  auto period(this const SlAtrNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this SlAtrNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  auto multiplier(this const SlAtrNode& self) noexcept -> const ErasedNode&
  {
    return self.multiplier_;
  }

  void multiplier(this SlAtrNode& self, ErasedNode multiplier) noexcept
  {
    self.multiplier_ = std::move(multiplier);
  }

  auto ma_smoothing_type(this const SlAtrNode& self) noexcept -> MaNodeType
  {
    return self.ma_smoothing_type_;
  }

  void ma_smoothing_type(this SlAtrNode& self, MaNodeType type) noexcept
  {
    self.ma_smoothing_type_ = type;
  }

private:
  ErasedNode period_;
  ErasedNode multiplier_;
  MaNodeType ma_smoothing_type_;
};

class TpAtrNode {
public:
  TpAtrNode()
  : TpAtrNode{14.0, 2.0}
  {
  }

  TpAtrNode(double period,
            double multiplier,
            MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : TpAtrNode{ValueNode{period}, ValueNode{multiplier}, ma_smoothing_type}
  {
  }

  TpAtrNode(ErasedNode period,
            ErasedNode multiplier,
            MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : period_{std::move(period)}
  , multiplier_{std::move(multiplier)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const TpAtrNode&) const noexcept -> bool = default;

  auto period(this const TpAtrNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this TpAtrNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  auto multiplier(this const TpAtrNode& self) noexcept -> const ErasedNode&
  {
    return self.multiplier_;
  }

  void multiplier(this TpAtrNode& self, ErasedNode multiplier) noexcept
  {
    self.multiplier_ = std::move(multiplier);
  }

  auto ma_smoothing_type(this const TpAtrNode& self) noexcept -> MaNodeType
  {
    return self.ma_smoothing_type_;
  }

  void ma_smoothing_type(this TpAtrNode& self, MaNodeType type) noexcept
  {
    self.ma_smoothing_type_ = type;
  }

private:
  ErasedNode period_;
  ErasedNode multiplier_;
  MaNodeType ma_smoothing_type_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const SlAmountNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   SlAmountMethod{node_to_erased_method(node.value(), context)}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const TpAmountNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   TpAmountMethod{node_to_erased_method(node.value(), context)}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const SlPercentNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   SlPercentMethod{node_to_erased_method(node.value(), context)}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const TpPercentNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   TpPercentMethod{node_to_erased_method(node.value(), context)}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const SlAtrNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   SlAtrMethod{node_to_erased_method(node.period(), context),
               node_to_erased_method(node.multiplier(), context),
               static_cast<MaMethodType>(node.ma_smoothing_type())}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const TpAtrNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   TpAtrMethod{node_to_erased_method(node.period(), context),
               node_to_erased_method(node.multiplier(), context),
               static_cast<MaMethodType>(node.ma_smoothing_type())}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const Sl1RNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{Sl1RMethod{}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const TpRMultipleNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   TpRMultipleMethod{node_to_erased_method(node.value(), context)}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const InitialEntryPriceNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   InitialEntryPriceMethod{}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const LatestEntryPriceNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   LatestEntryPriceMethod{}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const AveragePriceNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{AveragePriceMethod{}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const StopTargetRefPriceNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   StopTargetRefPriceMethod{}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const PositionDirectionNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   PositionDirectionMethod{}};
}

} // namespace pludux::backtest
