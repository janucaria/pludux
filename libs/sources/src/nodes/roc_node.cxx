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

  RocNode(std::size_t period)
  : RocNode{ErasedNode{}, period}
  {
  }

  RocNode(ErasedNode source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const RocNode& other) const noexcept -> bool = default;


  auto source(this const RocNode& self) noexcept -> ErasedNode
  {
    return self.source_;
  }

  void source(this RocNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RocNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this RocNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ErasedNode source_;
  std::size_t period_;
};

} // namespace pludux