module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.wma_node;

import :nodes.erased_node;

export namespace pludux {

class WmaNode {
public:
  WmaNode()
  : WmaNode{20}
  {
  }

  explicit WmaNode(std::size_t period)
  : WmaNode{ErasedNode{}, period}
  {
  }

  explicit WmaNode(ErasedNode source)
  : WmaNode{std::move(source), 20}
  {
  }

  WmaNode(ErasedNode source, std::size_t period)
  : WmaNode{std::move(source), ErasedNode{period}}
  {
  }

  WmaNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const WmaNode& other) const noexcept -> bool = default;

  auto source(this const WmaNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this WmaNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const WmaNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this WmaNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this WmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ErasedNode{period};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

} // namespace pludux
