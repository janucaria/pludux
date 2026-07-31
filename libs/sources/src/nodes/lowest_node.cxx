module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.lowest_node;

import :methods.lowest_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

class LowestNode {
public:
  LowestNode()
  : LowestNode{CloseNode{}, 14}
  {
  }

  explicit LowestNode(std::size_t period)
  : LowestNode{CloseNode{}, period}
  {
  }

  explicit LowestNode(ErasedNode<ErasedSeriesMethodContext> source)
  : LowestNode{std::move(source), 14}
  {
  }

  LowestNode(ErasedNode<ErasedSeriesMethodContext> source, std::size_t period)
  : LowestNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  LowestNode(ErasedNode<ErasedSeriesMethodContext> source,
             ErasedNode<ErasedSeriesMethodContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const LowestNode& other) const noexcept -> bool = default;

  auto source(this const LowestNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.source_;
  }

  void source(this LowestNode& self,
              ErasedNode<ErasedSeriesMethodContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const LowestNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.period_;
  }

  void period(this LowestNode& self,
              ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this LowestNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode<ErasedSeriesMethodContext> source_;
  ErasedNode<ErasedSeriesMethodContext> period_;
};

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const LowestNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return ErasedSeriesMethod<TContext>{LowestMethod{source_method, period}};
}

} // namespace pludux
