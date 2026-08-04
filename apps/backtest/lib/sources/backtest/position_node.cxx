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

class PositionRMultipleNode {
public:
  PositionRMultipleNode()
  : PositionRMultipleNode{CloseNode{}}
  {
  }

  explicit PositionRMultipleNode(ErasedNode<ErasedSeriesMethodContext> source)
  : source_{std::move(source)}
  {
  }

  auto operator==(const PositionRMultipleNode&) const noexcept
   -> bool = default;

  auto source(this const PositionRMultipleNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.source_;
  }

  void source(this PositionRMultipleNode& self,
              ErasedNode<ErasedSeriesMethodContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

private:
  ErasedNode<ErasedSeriesMethodContext> source_;
};

auto pludux_tag_invoke(NodeToErasedMethod<ErasedSeriesMethodContext>,
                       const InitialEntryPriceNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   InitialEntryPriceMethod{}};
}

auto pludux_tag_invoke(NodeToErasedMethod<ErasedSeriesMethodContext>,
                       const LatestEntryPriceNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   LatestEntryPriceMethod{}};
}

auto pludux_tag_invoke(NodeToErasedMethod<ErasedSeriesMethodContext>,
                       const AveragePriceNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{AveragePriceMethod{}};
}

auto pludux_tag_invoke(NodeToErasedMethod<ErasedSeriesMethodContext>,
                       const StopTargetRefPriceNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   StopTargetRefPriceMethod{}};
}

auto pludux_tag_invoke(NodeToErasedMethod<ErasedSeriesMethodContext>,
                       const PositionDirectionNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   PositionDirectionMethod{}};
}

auto pludux_tag_invoke(NodeToErasedMethod<ErasedSeriesMethodContext>,
                       const PositionRMultipleNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{PositionRMultipleMethod{
   node_to_erased_method<ErasedSeriesMethodContext>(node.source(), context)}};
}

} // namespace pludux::backtest
