module;

#include <cstddef>
#include <utility>
#include <variant>

export module pludux:nodes.donchian_channel_node;

import :methods.donchian_channel_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.value_node;

export namespace pludux {

template<typename TContext = std::monostate>
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

   explicit DonchianChannelNode(ErasedNode<TContext> period)
  : period_{std::move(period)}
  {
  }

  auto operator==(const DonchianChannelNode&) const noexcept -> bool = default;

  auto period(this const DonchianChannelNode& self) noexcept
    -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this DonchianChannelNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

  void period(this DonchianChannelNode& self,
               ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
   ErasedNode<TContext> period_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const DonchianChannelNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return ErasedSeriesMethod<TContext>{DonchianChannelMethod{period}};
}

} // namespace pludux
