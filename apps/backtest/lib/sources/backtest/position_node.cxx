module;

#include <utility>

export module pludux.backtest:position_node;

import pludux;

import :position_method;

export namespace pludux::backtest {

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

template<typename TContext>
class PositionRMultipleNode {
public:
  PositionRMultipleNode()
  : PositionRMultipleNode{CloseNode{}}
  {
  }

  explicit PositionRMultipleNode(ErasedNode<TContext> source)
  : source_{std::move(source)}
  {
  }

  auto operator==(const PositionRMultipleNode&) const noexcept
   -> bool = default;

  auto source(this const PositionRMultipleNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.source_;
  }

  void source(this PositionRMultipleNode& self,
              ErasedNode<TContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

private:
  ErasedNode<TContext> source_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const InitialEntryPriceNode&,
                        NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{InitialEntryPriceMethod{}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const LatestEntryPriceNode&,
                        NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{LatestEntryPriceMethod{}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const AveragePriceNode&,
                        NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{AveragePriceMethod{}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const StopTargetRefPriceNode&,
                        NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{StopTargetRefPriceMethod{}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const PositionDirectionNode&,
                        NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{PositionDirectionMethod{}};
}

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const PositionRMultipleNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{PositionRMultipleMethod{
   node_to_erased_method<TContext>(node.source(), context)}};
}

} // namespace pludux::backtest
