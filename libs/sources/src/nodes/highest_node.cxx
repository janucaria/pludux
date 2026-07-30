module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.highest_node;

import :methods.highest_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

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

  explicit HighestNode(ErasedNode source)
  : HighestNode{std::move(source), 14}
  {
  }

  HighestNode(ErasedNode source, std::size_t period)
  : HighestNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  HighestNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const HighestNode& other) const noexcept -> bool = default;

  auto source(this const HighestNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this HighestNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const HighestNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this HighestNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this HighestNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const HighestNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  const auto source_method = node_to_erased_method(node.source(), context);
  const auto period = node_to_erased_method(node.period(), context);

  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   HighestMethod{source_method, period}};
}

} // namespace pludux
