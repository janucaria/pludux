module;

#include <cstddef>
#include <limits>
#include <utility>
#include <variant>

export module pludux:nodes.rma_node;

import :methods.rma_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

template<typename TContext = std::monostate>
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

  explicit RmaNode(ErasedNode<TContext> source)
  : RmaNode{std::move(source), 20}
  {
  }

  RmaNode(ErasedNode<TContext> source, std::size_t period)
  : RmaNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  RmaNode(ErasedNode<TContext> source, ErasedNode<TContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const RmaNode& other) const noexcept -> bool = default;

  auto source(this const RmaNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.source_;
  }

  void source(this RmaNode& self,
               ErasedNode<TContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RmaNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this RmaNode& self,
               ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this RmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode<TContext> source_;
  ErasedNode<TContext> period_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const RmaNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return RmaMethod{source_method, period};
}

} // namespace pludux
