module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.stddev_node;

import :methods.stddev_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

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

  explicit StddevNode(ErasedNode source)
  : StddevNode{std::move(source), 20}
  {
  }

  StddevNode(ErasedNode source, std::size_t period)
  : StddevNode{std::move(source), ValueNode{static_cast<double>(period)}}
  {
  }

  StddevNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const StddevNode& other) const noexcept -> bool = default;

  auto source(this const StddevNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this StddevNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const StddevNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this StddevNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this StddevNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const StddevNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  const auto source_method = node_to_erased_method(node.source(), context);
  const auto period = node_to_erased_method(node.period(), context);

  return StddevMethod{source_method, period};
}

} // namespace pludux
