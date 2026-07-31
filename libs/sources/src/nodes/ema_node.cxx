module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.ema_node;

import :methods.ema_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

class EmaNode {
public:
  EmaNode()
  : EmaNode{CloseNode{}, 20}
  {
  }

  explicit EmaNode(ErasedNode<ErasedSeriesMethodContext> source)
  : EmaNode{std::move(source), 20}
  {
  }

  EmaNode(ErasedNode<ErasedSeriesMethodContext> source, std::size_t period)
  : EmaNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  EmaNode(ErasedNode<ErasedSeriesMethodContext> source,
          ErasedNode<ErasedSeriesMethodContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const EmaNode& other) const noexcept -> bool = default;

  auto source(this const EmaNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.source_;
  }

  void source(this EmaNode& self,
              ErasedNode<ErasedSeriesMethodContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const EmaNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.period_;
  }

  void period(this EmaNode& self,
              ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this EmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode<ErasedSeriesMethodContext> source_;
  ErasedNode<ErasedSeriesMethodContext> period_;
};

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const EmaNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return EmaMethod{source_method, period};
}

} // namespace pludux
