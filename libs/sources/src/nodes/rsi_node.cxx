module;

#include <concepts>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

export module pludux:nodes.rsi_node;

import :methods.rsi_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

template<typename TContext = std::monostate>
class RsiNode {
public:
  RsiNode()
  : RsiNode{CloseNode{}, 14}
  {
  }

  explicit RsiNode(std::size_t period)
  : RsiNode{CloseNode{}, period}
  {
  }

  explicit RsiNode(ErasedNode<TContext> source)
  : RsiNode{std::move(source), 14}
  {
  }

  RsiNode(ErasedNode<TContext> source, std::size_t period)
  : RsiNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  RsiNode(ErasedNode<TContext> source, ErasedNode<TContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const RsiNode& other) const noexcept -> bool = default;

  auto source(this const RsiNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.source_;
  }

  void source(this RsiNode& self,
               ErasedNode<TContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RsiNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this RsiNode& self,
               ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this RsiNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode<TContext> source_;
  ErasedNode<TContext> period_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const RsiNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return RsiMethod{source_method, period};
}

} // namespace pludux
