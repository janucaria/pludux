module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.sma_node;

import :methods.sma_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

class SmaNode {
public:
  SmaNode()
  : SmaNode{CloseNode{}, 20}
  {
  }

  explicit SmaNode(ErasedNode source)
  : SmaNode{std::move(source), 20}
  {
  }

  SmaNode(ErasedNode source, std::size_t period)
  : SmaNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  SmaNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const SmaNode& other) const noexcept -> bool = default;

  auto source(this const SmaNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this SmaNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const SmaNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this SmaNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this SmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const SmaNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  const auto source_method = node_to_erased_method(node.source(), context);
  const auto period = node_to_erased_method(node.period(), context);

  return SmaMethod{source_method, period};
}

} // namespace pludux
