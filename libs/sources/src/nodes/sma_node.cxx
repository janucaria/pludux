module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.sma_node;


import :nodes.erased_node;

export namespace pludux {

class SmaNode {
public:
  SmaNode()
  : SmaNode{20}
  {
  }

  explicit SmaNode(std::size_t period)
  : SmaNode{ErasedNode{}, period}
  {
  }

  explicit SmaNode(ErasedNode source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const SmaNode& other) const noexcept -> bool = default;


  auto source(this const SmaNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this SmaNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const SmaNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this SmaNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ErasedNode source_;
  std::size_t period_;
};

} // namespace pludux