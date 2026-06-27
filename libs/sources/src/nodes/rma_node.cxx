module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.rma_node;

import :nodes.erased_node;

export namespace pludux {

class RmaNode {
public:
  RmaNode()
  : RmaNode{20}
  {
  }

  explicit RmaNode(std::size_t period)
  : RmaNode{ErasedNode{}, period}
  {
  }

  explicit RmaNode(ErasedNode source)
  : RmaNode{std::move(source), 20}
  {
  }

  RmaNode(ErasedNode source, std::size_t period)
  : RmaNode{std::move(source), ErasedNode{period}}
  {
  }

  RmaNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const RmaNode& other) const noexcept -> bool = default;

  auto source(this const RmaNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this RmaNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RmaNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this RmaNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this RmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ErasedNode{period};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

} // namespace pludux
