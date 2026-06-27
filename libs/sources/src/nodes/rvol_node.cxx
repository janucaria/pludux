module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:nodes.rvol_node;

import :nodes.erased_node;

export namespace pludux {

class RvolNode {
public:
  explicit RvolNode(std::size_t period = 14)
  : RvolNode{ErasedNode{period}}
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
    self.period_ = ErasedNode{period};
  }

  void period(this RvolNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  ErasedNode period_;
};

} // namespace pludux
