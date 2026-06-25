module;

#include <utility>

export module pludux:nodes.crossunder_node;

import :nodes.erased_node;

export namespace pludux {

class CrossunderNode {
public:
  CrossunderNode(ErasedNode source, ErasedNode reference)
  : source_{std::move(source)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossunderNode& other) const noexcept
   -> bool = default;


  auto source(this const CrossunderNode& self) noexcept
   -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this CrossunderNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto reference(this const CrossunderNode& self) noexcept
   -> const ErasedNode&
  {
    return self.reference_;
  }

  void reference(this CrossunderNode& self,
                 ErasedNode reference) noexcept
  {
    self.reference_ = std::move(reference);
  }

private:
  ErasedNode source_;
  ErasedNode reference_;
};

} // namespace pludux