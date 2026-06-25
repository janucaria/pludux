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

  explicit RmaNode(ErasedNode source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
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

  auto period(this const RmaNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this RmaNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ErasedNode source_;
  std::size_t period_;
};

} // namespace pludux