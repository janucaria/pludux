module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.stoch_node;


export namespace pludux {

class StochNode {
public:
  StochNode(std::size_t k_period, std::size_t k_smooth, std::size_t d_period)
  : k_period_{k_period}
  , k_smooth_{k_smooth}
  , d_period_{d_period}
  {
  }

  auto operator==(const StochNode& other) const noexcept -> bool = default;


  auto k_period(this const StochNode& self) noexcept -> std::size_t
  {
    return self.k_period_;
  }

  void k_period(this StochNode& self, std::size_t k_period) noexcept
  {
    self.k_period_ = k_period;
  }

  auto k_smooth(this const StochNode& self) noexcept -> std::size_t
  {
    return self.k_smooth_;
  }

  void k_smooth(this StochNode& self, std::size_t k_smooth) noexcept
  {
    self.k_smooth_ = k_smooth;
  }

  auto d_period(this const StochNode& self) noexcept -> std::size_t
  {
    return self.d_period_;
  }

  void d_period(this StochNode& self, std::size_t d_period) noexcept
  {
    self.d_period_ = d_period;
  }

private:
  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
};

} // namespace pludux