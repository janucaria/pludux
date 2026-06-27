module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.roc_node;

import :nodes.erased_node;

export namespace pludux {

class RocNode {
public:
  RocNode()
  : RocNode{14}
  {
  }

  explicit RocNode(std::size_t period)
  : RocNode{ErasedNode{}, period}
  {
  }

  explicit RocNode(ErasedNode source)
  : RocNode{std::move(source), 14}
  {
  }

  RocNode(ErasedNode source, std::size_t period)
  : RocNode{std::move(source), ErasedNode{period}}
  {
  }

  RocNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const RocNode& other) const noexcept -> bool = default;

  auto source(this const RocNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this RocNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RocNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this RocNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this RocNode& self, std::size_t period) noexcept
  {
    self.period_ = ErasedNode{period};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

} // namespace pludux
