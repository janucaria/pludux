module;

#include <utility>

export module pludux:nodes.crossover_node;

import :nodes.erased_node;

export namespace pludux {

class CrossoverNode {
public:
  CrossoverNode(ErasedNode source, ErasedNode reference)
  : source_{std::move(source)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossoverNode& other) const noexcept
   -> bool = default;


  auto source(this const CrossoverNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this CrossoverNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto reference(this const CrossoverNode& self) noexcept
   -> const ErasedNode&
  {
    return self.reference_;
  }

  void reference(this CrossoverNode& self,
                 ErasedNode reference) noexcept
  {
    self.reference_ = std::move(reference);
  }

private:
  ErasedNode source_;
  ErasedNode reference_;
};

} // namespace pludux