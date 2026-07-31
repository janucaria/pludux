module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.roc_node;

import :methods.roc_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

class RocNode {
public:
  RocNode()
  : RocNode{CloseNode{}, 14}
  {
  }

  explicit RocNode(std::size_t period)
  : RocNode{CloseNode{}, period}
  {
  }

  explicit RocNode(ErasedNode<ErasedSeriesMethodContext> source)
  : RocNode{std::move(source), 14}
  {
  }

  RocNode(ErasedNode<ErasedSeriesMethodContext> source, std::size_t period)
  : RocNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  RocNode(ErasedNode<ErasedSeriesMethodContext> source,
          ErasedNode<ErasedSeriesMethodContext> period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const RocNode& other) const noexcept -> bool = default;

  auto source(this const RocNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.source_;
  }

  void source(this RocNode& self,
              ErasedNode<ErasedSeriesMethodContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RocNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.period_;
  }

  void period(this RocNode& self,
              ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this RocNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode<ErasedSeriesMethodContext> source_;
  ErasedNode<ErasedSeriesMethodContext> period_;
};

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const RocNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return RocMethod{source_method, period};
}

} // namespace pludux
