module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.rma_node;

import :methods.rma_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

class RmaNode {
public:
  RmaNode()
  : RmaNode{CloseNode{}, 20}
  {
  }

  explicit RmaNode(std::size_t period)
  : RmaNode{CloseNode{}, period}
  {
  }

  explicit RmaNode(ErasedNode<ErasedSeriesMethodContext> source)
  : RmaNode{std::move(source), 20}
  {
  }

  RmaNode(ErasedNode<ErasedSeriesMethodContext> source, std::size_t period)
  : RmaNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  RmaNode(ErasedNode<ErasedSeriesMethodContext> source,
          ErasedNode<ErasedSeriesMethodContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const RmaNode& other) const noexcept -> bool = default;

  auto source(this const RmaNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.source_;
  }

  void source(this RmaNode& self,
              ErasedNode<ErasedSeriesMethodContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RmaNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.period_;
  }

  void period(this RmaNode& self,
              ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this RmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode<ErasedSeriesMethodContext> source_;
  ErasedNode<ErasedSeriesMethodContext> period_;
};

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const RmaNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return RmaMethod{source_method, period};
}

} // namespace pludux
