module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.highest_node;


import :nodes.erased_node;

export namespace pludux {

class HighestNode {
public:
  HighestNode()
  : HighestNode{14}
  {
  }

  explicit HighestNode(std::size_t period)
  : HighestNode{ErasedNode{}, period}
  {
  }

  explicit HighestNode(ErasedNode source)
  : HighestNode{std::move(source), 14}
  {
  }

  HighestNode(ErasedNode source, std::size_t period)
  : HighestNode{std::move(source), ErasedNode{period}}
  {
  }

  HighestNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const HighestNode& other) const noexcept -> bool = default;


  auto source(this const HighestNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this HighestNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const HighestNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this HighestNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this HighestNode& self, std::size_t period) noexcept
  {
    self.period_ = ErasedNode{period};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

} // namespace pludux
