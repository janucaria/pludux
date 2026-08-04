module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.hma_node;

import :methods.hma_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

class HmaNode {
public:
  HmaNode()
  : HmaNode{CloseNode{}, 20}
  {
  }

  explicit HmaNode(std::size_t period)
  : HmaNode{CloseNode{}, period}
  {
  }

  explicit HmaNode(ErasedNode<ErasedSeriesMethodContext> source)
  : HmaNode{std::move(source), 20}
  {
  }

  HmaNode(ErasedNode<ErasedSeriesMethodContext> source, std::size_t period)
  : HmaNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  HmaNode(ErasedNode<ErasedSeriesMethodContext> source,
          ErasedNode<ErasedSeriesMethodContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const HmaNode& other) const noexcept -> bool = default;

  auto source(this const HmaNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.source_;
  }

  void source(this HmaNode& self,
              ErasedNode<ErasedSeriesMethodContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const HmaNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.period_;
  }

  void period(this HmaNode& self,
              ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this HmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode<ErasedSeriesMethodContext> source_;
  ErasedNode<ErasedSeriesMethodContext> period_;
};

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const HmaNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return HmaMethod{source_method, period};
}

} // namespace pludux
