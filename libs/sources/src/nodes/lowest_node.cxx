module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.lowest_node;


import :nodes.erased_node;

export namespace pludux {

class LowestNode {
public:
  LowestNode()
  : LowestNode{14}
  {
  }

  explicit LowestNode(std::size_t period)
  : LowestNode{ErasedNode{}, period}
  {
  }

  explicit LowestNode(ErasedNode node, std::size_t period)
  : source_{std::move(node)}
  , period_{period}
  {
  }

  auto operator==(const LowestNode& other) const noexcept -> bool = default;


  auto source(this const LowestNode& self) -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this LowestNode& self, ErasedNode node)
  {
    self.source_ = std::move(node);
  }

  auto period(this const LowestNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this LowestNode& self, std::size_t period)
  {
    self.period_ = period;
  }

private:
  ErasedNode source_;
  std::size_t period_;
};

} // namespace pludux