module;

#include <utility>

export module pludux:nodes.change_node;

import :nodes.erased_node;

export namespace pludux {

class ChangeNode {
public:
  ChangeNode()
  : ChangeNode{ErasedNode{}}
  {
  }

  explicit ChangeNode(ErasedNode source)
  : source_{std::move(source)}
  {
  }

  auto operator==(const ChangeNode& other) const noexcept -> bool = default;


  auto source(this const ChangeNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this ChangeNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

private:
  ErasedNode source_;
};

} // namespace pludux