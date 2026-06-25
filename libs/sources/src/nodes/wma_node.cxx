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

  explicit WmaNode(ErasedNode source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
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

  auto period(this const WmaNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this WmaNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ErasedNode source_;
  std::size_t period_;
};

} // namespace pludux