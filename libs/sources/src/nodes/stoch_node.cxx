module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.stoch_node;

import :nodes.erased_node;

export namespace pludux {

class StochNode {
public:
  StochNode(std::size_t k_period, std::size_t k_smooth, std::size_t d_period)
  : StochNode{ErasedNode{k_period}, ErasedNode{k_smooth}, ErasedNode{d_period}}
  {
  }

  StochNode(ErasedNode k_period, ErasedNode k_smooth, ErasedNode d_period)
  : k_period_{std::move(k_period)}
  , k_smooth_{std::move(k_smooth)}
  , d_period_{std::move(d_period)}
  {
  }

  auto operator==(const StochNode& other) const noexcept -> bool = default;

  auto k_period(this const StochNode& self) noexcept -> const ErasedNode&
  {
    return self.k_period_;
  }

  void k_period(this StochNode& self, std::size_t k_period) noexcept
  {
    self.k_period_ = ErasedNode{k_period};
  }

  void k_period(this StochNode& self, ErasedNode k_period) noexcept
  {
    self.k_period_ = std::move(k_period);
  }

  auto k_smooth(this const StochNode& self) noexcept -> const ErasedNode&
  {
    return self.k_smooth_;
  }

  void k_smooth(this StochNode& self, std::size_t k_smooth) noexcept
  {
    self.k_smooth_ = ErasedNode{k_smooth};
  }

  void k_smooth(this StochNode& self, ErasedNode k_smooth) noexcept
  {
    self.k_smooth_ = std::move(k_smooth);
  }

  auto d_period(this const StochNode& self) noexcept -> const ErasedNode&
  {
    return self.d_period_;
  }

  void d_period(this StochNode& self, std::size_t d_period) noexcept
  {
    self.d_period_ = ErasedNode{d_period};
  }

  void d_period(this StochNode& self, ErasedNode d_period) noexcept
  {
    self.d_period_ = std::move(d_period);
  }

private:
  ErasedNode k_period_;
  ErasedNode k_smooth_;
  ErasedNode d_period_;
};

} // namespace pludux
