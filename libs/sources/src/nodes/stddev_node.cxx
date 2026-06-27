module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.stddev_node;

import :nodes.erased_node;

export namespace pludux {

class StddevNode {
public:
  StddevNode()
  : StddevNode{20}
  {
  }

  explicit StddevNode(std::size_t period)
  : StddevNode{ErasedNode{}, period}
  {
  }

  explicit StddevNode(ErasedNode source)
  : StddevNode{std::move(source), 20}
  {
  }

  StddevNode(ErasedNode source, std::size_t period)
  : StddevNode{std::move(source), ErasedNode{period}}
  {
  }

  StddevNode(ErasedNode source, ErasedNode period)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const StddevNode& other) const noexcept -> bool = default;

  auto source(this const StddevNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this StddevNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const StddevNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this StddevNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  void period(this StddevNode& self, std::size_t period) noexcept
  {
    self.period_ = ErasedNode{period};
  }

private:
  ErasedNode source_;
  ErasedNode period_;
};

} // namespace pludux
