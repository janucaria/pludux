module;

#include <cstddef>
#include <utility>

export module pludux:nodes.donchian_channel_node;

import :methods.donchian_channel_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.value_node;

export namespace pludux {

class DonchianChannelNode {
public:
  DonchianChannelNode()
  : DonchianChannelNode{20}
  {
  }

  DonchianChannelNode(std::size_t period)
  : DonchianChannelNode{ValueNode{static_cast<double>(period)}}
  {
  }

  explicit DonchianChannelNode(ErasedNode period)
  : period_{std::move(period)}
  {
  }

  auto operator==(const DonchianChannelNode&) const noexcept -> bool = default;

  auto period(this const DonchianChannelNode& self) noexcept
   -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this DonchianChannelNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

  void period(this DonchianChannelNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  ErasedNode period_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const DonchianChannelNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  const auto period = node_to_erased_method(node.period(), context);

  return AnySeriesMethod{DonchianChannelMethod{period}};
}

} // namespace pludux
