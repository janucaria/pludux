module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.stoch_node;

import :methods.stoch_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.value_node;

export namespace pludux {

class StochNode {
public:
  StochNode(std::size_t k_period, std::size_t k_smooth, std::size_t d_period)
  : StochNode{ValueNode{static_cast<double>(k_period)},
              ValueNode{static_cast<double>(k_smooth)},
              ValueNode{static_cast<double>(d_period)}}
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
    self.k_period_ = ValueNode{static_cast<double>(k_period)};
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
    self.k_smooth_ = ValueNode{static_cast<double>(k_smooth)};
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
    self.d_period_ = ValueNode{static_cast<double>(d_period)};
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

auto pludux_tag_invoke(NodeToErasedMethod,
                       const StochNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  const auto k_period = node_to_erased_method(node.k_period(), context);
  const auto k_smooth = node_to_erased_method(node.k_smooth(), context);
  const auto d_period = node_to_erased_method(node.d_period(), context);

  return StochMethod{k_period, k_smooth, d_period};
}

} // namespace pludux
