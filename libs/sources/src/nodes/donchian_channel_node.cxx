module;

#include <cstddef>
#include <utility>

export module pludux:nodes.donchian_channel_node;

import :nodes.erased_node;

export namespace pludux {

class DonchianChannelNode {
public:
  DonchianChannelNode()
  : DonchianChannelNode{20}
  {
  }

  DonchianChannelNode(std::size_t period)
  : DonchianChannelNode{ErasedNode{period}}
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
    self.period_ = ErasedNode{period};
  }

  void period(this DonchianChannelNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  ErasedNode period_;
};

} // namespace pludux
