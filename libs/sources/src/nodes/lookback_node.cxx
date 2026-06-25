module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.lookback_node;

import :nodes.erased_node;

export namespace pludux {

class LookbackNode {
public:
  explicit LookbackNode(std::size_t period = 1)
  : LookbackNode{ErasedNode{}, period}
  {
  }

  LookbackNode(ErasedNode source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  LookbackNode(const LookbackNode& other,
                 std::size_t additional_period)
  : LookbackNode{other.source(), other.period() + additional_period}
  {
  }

  auto operator==(const LookbackNode& other) const noexcept -> bool = default;


  auto source(this const LookbackNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this LookbackNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const LookbackNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this LookbackNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ErasedNode source_;
  std::size_t period_;
};

} // namespace pludux