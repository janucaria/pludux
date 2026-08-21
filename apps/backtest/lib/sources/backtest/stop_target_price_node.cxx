module;

#include <utility>

export module pludux.backtest:stop_target_price_node;

import pludux;

import :stop_target_price_method;

export namespace pludux::backtest {

template<typename TContext>
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

  explicit StopTargetDistanceNode(ErasedNode<TContext> value)
  : value_{std::move(value)}
  {
  }

  auto operator==(const StopTargetDistanceNode&) const noexcept
   -> bool = default;

  auto value(this const StopTargetDistanceNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.value_;
  }

  void value(this StopTargetDistanceNode& self,
             ErasedNode<TContext> value) noexcept
  {
    self.value_ = std::move(value);
  }

private:
  ErasedNode<TContext> value_;
};

template<typename TContext>
class SlAmountNode : public StopTargetDistanceNode<TContext> {
public:
  using StopTargetDistanceNode<TContext>::StopTargetDistanceNode;

  auto operator==(const SlAmountNode&) const noexcept -> bool = default;
};

template<typename TContext>
class TpAmountNode : public StopTargetDistanceNode<TContext> {
public:
  using StopTargetDistanceNode<TContext>::StopTargetDistanceNode;

  auto operator==(const TpAmountNode&) const noexcept -> bool = default;
};

template<typename TContext>
class SlPercentNode : public StopTargetDistanceNode<TContext> {
public:
  using StopTargetDistanceNode<TContext>::StopTargetDistanceNode;

  auto operator==(const SlPercentNode&) const noexcept -> bool = default;
};

template<typename TContext>
class TpPercentNode : public StopTargetDistanceNode<TContext> {
public:
  using StopTargetDistanceNode<TContext>::StopTargetDistanceNode;

  auto operator==(const TpPercentNode&) const noexcept -> bool = default;
};

template<typename TContext>
class SlRMultipleNode : public StopTargetDistanceNode<TContext> {
public:
  SlRMultipleNode()
  : StopTargetDistanceNode<TContext>{1.0}
  {
  }

  using StopTargetDistanceNode<TContext>::StopTargetDistanceNode;

  auto operator==(const SlRMultipleNode&) const noexcept -> bool = default;
};

class Sl1RNode {
public:
  auto operator==(const Sl1RNode&) const noexcept -> bool = default;
};

template<typename TContext>
class TpRMultipleNode : public StopTargetDistanceNode<TContext> {
public:
  TpRMultipleNode()
  : StopTargetDistanceNode<TContext>{2.0}
  {
  }

  using StopTargetDistanceNode<TContext>::StopTargetDistanceNode;

  auto operator==(const TpRMultipleNode&) const noexcept -> bool = default;
};

template<typename TContext>
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

  SlAtrNode(ErasedNode<TContext> period,
                 ErasedNode<TContext> multiplier,
                 MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : period_{std::move(period)}
  , multiplier_{std::move(multiplier)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const SlAtrNode&) const noexcept -> bool = default;

  auto period(this const SlAtrNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this SlAtrNode& self, ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  auto multiplier(this const SlAtrNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.multiplier_;
  }

  void multiplier(this SlAtrNode& self,
                  ErasedNode<TContext> multiplier) noexcept
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
  ErasedNode<TContext> period_;
  ErasedNode<TContext> multiplier_;
  MaNodeType ma_smoothing_type_;
};

template<typename TContext>
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

  TpAtrNode(ErasedNode<TContext> period,
                 ErasedNode<TContext> multiplier,
                 MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : period_{std::move(period)}
  , multiplier_{std::move(multiplier)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const TpAtrNode&) const noexcept -> bool = default;

  auto period(this const TpAtrNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this TpAtrNode& self, ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  auto multiplier(this const TpAtrNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.multiplier_;
  }

  void multiplier(this TpAtrNode& self,
                  ErasedNode<TContext> multiplier) noexcept
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
  ErasedNode<TContext> period_;
  ErasedNode<TContext> multiplier_;
  MaNodeType ma_smoothing_type_;
};


template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const SlAmountNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{SlAmountMethod{
   node_to_erased_method<TContext>(node.value(), context)}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const TpAmountNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{TpAmountMethod{
   node_to_erased_method<TContext>(node.value(), context)}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const SlPercentNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{SlPercentMethod{
   node_to_erased_method<TContext>(node.value(), context)}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const TpPercentNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{TpPercentMethod{
   node_to_erased_method<TContext>(node.value(), context)}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const SlAtrNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{SlAtrMethod{
    node_to_erased_method<TContext>(node.period(), context),
    node_to_erased_method<TContext>(node.multiplier(), context),
   static_cast<MaMethodType>(node.ma_smoothing_type())}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const TpAtrNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{TpAtrMethod{
    node_to_erased_method<TContext>(node.period(), context),
    node_to_erased_method<TContext>(node.multiplier(), context),
   static_cast<MaMethodType>(node.ma_smoothing_type())}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const SlRMultipleNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{SlRMultipleMethod{
   node_to_erased_method<TContext>(node.value(), context)}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const Sl1RNode&,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return node_to_erased_method<TContext>(SlRMultipleNode<TContext>{1.0},
                                         context);
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const TpRMultipleNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{TpRMultipleMethod{
   node_to_erased_method<TContext>(node.value(), context)}};
}

} // namespace pludux::backtest
