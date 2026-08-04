module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.sma_node;

import :methods.sma_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

class SmaNode {
public:
  SmaNode()
  : SmaNode{CloseNode{}, 20}
  {
  }

  explicit SmaNode(ErasedNode<ErasedSeriesMethodContext> source)
  : SmaNode{std::move(source), 20}
  {
  }

  SmaNode(ErasedNode<ErasedSeriesMethodContext> source, std::size_t period)
  : SmaNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  SmaNode(ErasedNode<ErasedSeriesMethodContext> source,
          ErasedNode<ErasedSeriesMethodContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const SmaNode& other) const noexcept -> bool = default;

  auto source(this const SmaNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.source_;
  }

  void source(this SmaNode& self,
              ErasedNode<ErasedSeriesMethodContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const SmaNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.period_;
  }

  void period(this SmaNode& self,
              ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this SmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode<ErasedSeriesMethodContext> source_;
  ErasedNode<ErasedSeriesMethodContext> period_;
};

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const SmaNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return SmaMethod{source_method, period};
}

} // namespace pludux
