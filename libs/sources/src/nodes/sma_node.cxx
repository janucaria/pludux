module;

#include <cstddef>
#include <limits>
#include <utility>
#include <variant>

export module pludux:nodes.sma_node;

import :methods.sma_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

template<typename TContext = std::monostate>
class SmaNode {
public:
   SmaNode()
   : SmaNode{CloseNode{}, 20}
  {
  }

   explicit SmaNode(ErasedNode<TContext> source)
   : SmaNode{std::move(source), 20}
  {
  }

   SmaNode(ErasedNode<TContext> source, std::size_t period)
   : SmaNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

   SmaNode(ErasedNode<TContext> source, ErasedNode<TContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

   auto operator==(const SmaNode& other) const noexcept -> bool = default;

   auto source(this const SmaNode& self) noexcept
    -> const ErasedNode<TContext>&
  {
    return self.source_;
  }

   void source(this SmaNode& self, ErasedNode<TContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

   auto period(this const SmaNode& self) noexcept
    -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

   void period(this SmaNode& self, ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

   void period(this SmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
   ErasedNode<TContext> source_;
   ErasedNode<TContext> period_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const SmaNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return SmaMethod{source_method, period};
}

} // namespace pludux
