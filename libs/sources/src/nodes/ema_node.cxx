module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.ema_node;


import :nodes.erased_node;

export namespace pludux {

class EmaNode {
public:
  EmaNode()
  : EmaNode{20}
  {
  }

  EmaNode(std::size_t period)
  : EmaNode{ErasedNode{}, period}
  {
  }

  EmaNode(ErasedNode source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const EmaNode& other) const noexcept -> bool = default;


  auto source(this const EmaNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this EmaNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const EmaNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this EmaNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ErasedNode source_;
  std::size_t period_;
};

} // namespace pludux