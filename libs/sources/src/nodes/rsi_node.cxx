module;

#include <concepts>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

export module pludux:nodes.rsi_node;

import :nodes.erased_node;

export namespace pludux {

class RsiNode {
public:
  RsiNode()
  : RsiNode{14}
  {
  }

  explicit RsiNode(std::size_t period)
  : RsiNode{ErasedNode{}, period}
  {
  }

  explicit RsiNode(ErasedNode source)
  : RsiNode{std::move(source), 14}
  {
  }

  RsiNode(ErasedNode source, std::size_t period)
  : RsiNode{std::move(source), ErasedNode{period}}
  {
  }

  RsiNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const RsiNode& other) const noexcept -> bool = default;

  auto source(this const RsiNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this RsiNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RsiNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this RsiNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this RsiNode& self, std::size_t period) noexcept
  {
    self.period_ = ErasedNode{period};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

} // namespace pludux
