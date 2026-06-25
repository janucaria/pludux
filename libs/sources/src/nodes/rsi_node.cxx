module;

#include <concepts>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

export module pludux:nodes.rsi_node;

import :nodes.erased_node;
import :nodes.value_node;

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

  explicit RsiNode(ErasedNode source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
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

  auto period(this const RsiNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this RsiNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ErasedNode source_;
  std::size_t period_;
};

} // namespace pludux