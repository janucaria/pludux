module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.hma_node;

import :nodes.erased_node;

export namespace pludux {

class HmaNode {
public:
  HmaNode()
  : HmaNode{20}
  {
  }

  explicit HmaNode(std::size_t period)
  : HmaNode{ErasedNode{}, period}
  {
  }

  explicit HmaNode(ErasedNode source)
  : HmaNode{std::move(source), 20}
  {
  }

  HmaNode(ErasedNode source, std::size_t period)
  : HmaNode{std::move(source), ErasedNode{period}}
  {
  }

  HmaNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const HmaNode& other) const noexcept -> bool = default;

  auto source(this const HmaNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this HmaNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const HmaNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this HmaNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this HmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ErasedNode{period};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

} // namespace pludux
