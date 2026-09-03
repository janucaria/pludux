module;

#include <cstddef>
#include <limits>
#include <utility>
#include <variant>

export module pludux:nodes.highest_node;

import :methods.highest_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

template<typename TContext = std::monostate>
class HighestNode {
public:
  HighestNode()
  : HighestNode{CloseNode{}, 14}
  {
  }

  explicit HighestNode(std::size_t period)
  : HighestNode{CloseNode{}, period}
  {
  }

  explicit HighestNode(ErasedNode<TContext> source)
  : HighestNode{std::move(source), 14}
  {
  }

  HighestNode(ErasedNode<TContext> source, std::size_t period)
  : HighestNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  HighestNode(ErasedNode<TContext> source, ErasedNode<TContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const HighestNode& other) const noexcept -> bool = default;

  auto source(this const HighestNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.source_;
  }

  void source(this HighestNode& self,
               ErasedNode<TContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const HighestNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this HighestNode& self,
               ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this HighestNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode<TContext> source_;
  ErasedNode<TContext> period_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const HighestNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return ErasedSeriesMethod<TContext>{HighestMethod{source_method, period}};
}

} // namespace pludux
