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

  explicit EmaNode(ErasedNode source)
  : EmaNode{std::move(source), 20}
  {
  }

  EmaNode(ErasedNode source, std::size_t period)
  : EmaNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  EmaNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const EmaNode& other) const noexcept -> bool = default;

  auto source(this const EmaNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this EmaNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const EmaNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this EmaNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this EmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const EmaNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  const auto source_method = node_to_erased_method(node.source(), context);
  const auto period = node_to_erased_method(node.period(), context);

  return EmaMethod{source_method, period};
}

} // namespace pludux
