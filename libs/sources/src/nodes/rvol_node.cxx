module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:nodes.rvol_node;

import :methods.rvol_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.value_node;

export namespace pludux {

class RvolNode {
public:
  explicit RvolNode(std::size_t period = 14)
  : RvolNode{ValueNode{static_cast<double>(period)}}
  {
  }

  explicit RvolNode(ErasedNode period)
  : period_{std::move(period)}
  {
  }

  auto operator==(const RvolNode& other) const noexcept -> bool = default;

  auto period(this const RvolNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this RvolNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

  void period(this RvolNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  ErasedNode period_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const RvolNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  const auto period = node_to_erased_method(node.period(), context);

  return AnySeriesMethod{RvolMethod{period}};
}

} // namespace pludux
