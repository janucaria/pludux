module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.highest_node;


import :nodes.erased_node;

export namespace pludux {

class HighestNode {
public:
  HighestNode()
  : HighestNode{14}
  {
  }

  explicit HighestNode(std::size_t period)
  : HighestNode{ErasedNode{}, period}
  {
  }

  explicit HighestNode(ErasedNode node, std::size_t period)
  : source_{std::move(node)}
  , period_{period}
  {
  }

  auto operator==(const HighestNode& other) const noexcept -> bool = default;


  auto source(this const HighestNode& self) -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this HighestNode& self, ErasedNode node)
  {
    self.source_ = std::move(node);
  }

  auto period(this const HighestNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this HighestNode& self, std::size_t period)
  {
    self.period_ = period;
  }

private:
  ErasedNode source_;
  std::size_t period_;
};

} // namespace pludux