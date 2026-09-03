module;

#include <cstddef>
#include <limits>
#include <utility>
#include <variant>

export module pludux:nodes.wma_node;

import :methods.wma_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

template<typename TContext = std::monostate>
class WmaNode {
public:
  WmaNode()
  : WmaNode{CloseNode{}, 20}
  {
  }

  explicit WmaNode(std::size_t period)
  : WmaNode{CloseNode{}, period}
  {
  }

  explicit WmaNode(ErasedNode<TContext> source)
  : WmaNode{std::move(source), 20}
  {
  }

  WmaNode(ErasedNode<TContext> source, std::size_t period)
  : WmaNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  WmaNode(ErasedNode<TContext> source, ErasedNode<TContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const WmaNode& other) const noexcept -> bool = default;

  auto source(this const WmaNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.source_;
  }

  void source(this WmaNode& self,
               ErasedNode<TContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const WmaNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this WmaNode& self,
               ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this WmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode<TContext> source_;
  ErasedNode<TContext> period_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const WmaNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return WmaMethod{source_method, period};
}

} // namespace pludux
