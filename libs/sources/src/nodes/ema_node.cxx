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

  explicit EmaNode(ErasedNode source)
  : EmaNode{std::move(source), 20}
  {
  }

  EmaNode(ErasedNode source, std::size_t period)
  : EmaNode{std::move(source), ErasedNode{period}}
  {
  }

  EmaNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
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

  auto period(this const EmaNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this EmaNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this EmaNode& self, std::size_t period) noexcept
  {
    self.period_ = ErasedNode{period};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

} // namespace pludux
