module;

#include <cstddef>

export module pludux:nodes.donchian_channel_node;


export namespace pludux {

class DonchianChannelNode {
public:
  DonchianChannelNode()
  : DonchianChannelNode{20}
  {
  }

  DonchianChannelNode(std::size_t period)
  : period_{period}
  {
  }

  auto operator==(const DonchianChannelNode&) const noexcept
   -> bool = default;


  auto period(this const DonchianChannelNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this DonchianChannelNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  std::size_t period_;
};

} // namespace pludux