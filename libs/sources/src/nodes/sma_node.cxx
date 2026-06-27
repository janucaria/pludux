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

  explicit SmaNode(ErasedNode source)
  : SmaNode{std::move(source), 20}
  {
  }

  SmaNode(ErasedNode source, std::size_t period)
  : SmaNode{std::move(source), ErasedNode{period}}
  {
  }

  SmaNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
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

  auto period(this const SmaNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this SmaNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this SmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ErasedNode{period};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

} // namespace pludux
