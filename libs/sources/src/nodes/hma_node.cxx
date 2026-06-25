module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.hma_node;


import :nodes.erased_node;
import :nodes.value_node;
import :nodes.wma_node;
import :nodes.operators_node;

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

  explicit HmaNode(ErasedNode source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
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

  auto period(this const HmaNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this HmaNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ErasedNode source_;
  std::size_t period_;
};

} // namespace pludux