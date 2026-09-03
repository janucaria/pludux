module;

#include <cstddef>
#include <limits>
#include <utility>
#include <variant>

export module pludux:nodes.stddev_node;

import :methods.stddev_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

template<typename TContext = std::monostate>
class StddevNode {
public:
  StddevNode()
  : StddevNode{CloseNode{}, 20}
  {
  }

  explicit StddevNode(std::size_t period)
  : StddevNode{CloseNode{}, period}
  {
  }

  explicit StddevNode(ErasedNode<TContext> source)
  : StddevNode{std::move(source), 20}
  {
  }

  StddevNode(ErasedNode<TContext> source, std::size_t period)
  : StddevNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  StddevNode(ErasedNode<TContext> source, ErasedNode<TContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const StddevNode& other) const noexcept -> bool = default;

  auto source(this const StddevNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.source_;
  }

  void source(this StddevNode& self,
               ErasedNode<TContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const StddevNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this StddevNode& self,
               ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this StddevNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode<TContext> source_;
  ErasedNode<TContext> period_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const StddevNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return StddevMethod{source_method, period};
}

} // namespace pludux
