module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.rma_node;

import :methods.rma_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

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

  explicit RmaNode(ErasedNode source)
  : RmaNode{std::move(source), 20}
  {
  }

  RmaNode(ErasedNode source, std::size_t period)
  : RmaNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  RmaNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const RmaNode& other) const noexcept -> bool = default;

  auto source(this const RmaNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this RmaNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RmaNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this RmaNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this RmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const RmaNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  const auto source_method = node_to_erased_method(node.source(), context);
  const auto period = node_to_erased_method(node.period(), context);

  return RmaMethod{source_method, period};
}

} // namespace pludux
