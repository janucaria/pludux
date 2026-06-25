module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.macd_node;

import :nodes.erased_node;

export namespace pludux {

class MacdNode {
public:
  MacdNode()
  : MacdNode{12, 26, 9}
  {
  }

  MacdNode(std::size_t short_period,
             std::size_t long_period,
             std::size_t signal_period)
  : MacdNode{ErasedNode{}, short_period, long_period, signal_period}
  {
  }

  MacdNode(ErasedNode source,
             std::size_t short_period,
             std::size_t long_period,
             std::size_t signal_period)
  : source_{std::move(source)}
  , short_period_{short_period}
  , long_period_{long_period}
  , signal_period_{signal_period}
  {
  }

  auto operator==(const MacdNode& other) const noexcept -> bool = default;


  auto source(this const MacdNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this MacdNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto short_period(this const MacdNode& self) noexcept -> std::size_t
  {
    return self.short_period_;
  }

  void short_period(this MacdNode& self, std::size_t period) noexcept
  {
    self.short_period_ = period;
  }

  auto fast_period(this const MacdNode& self) noexcept -> std::size_t
  {
    return self.short_period_;
  }

  void fast_period(this MacdNode& self, std::size_t period) noexcept
  {
    self.short_period_ = period;
  }

  auto long_period(this const MacdNode& self) noexcept -> std::size_t
  {
    return self.long_period_;
  }

  void long_period(this MacdNode& self, std::size_t period) noexcept
  {
    self.long_period_ = period;
  }

  auto slow_period(this const MacdNode& self) noexcept -> std::size_t
  {
    return self.long_period_;
  }

  void slow_period(this MacdNode& self, std::size_t period) noexcept
  {
    self.long_period_ = period;
  }

  auto signal_period(this const MacdNode& self) noexcept -> std::size_t
  {
    return self.signal_period_;
  }

  void signal_period(this MacdNode& self, std::size_t period) noexcept
  {
    self.signal_period_ = period;
  }

private:
  ErasedNode source_;
  std::size_t short_period_;
  std::size_t long_period_;
  std::size_t signal_period_;
};

} // namespace pludux