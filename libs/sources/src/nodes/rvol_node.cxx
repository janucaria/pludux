module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:nodes.rvol_node;

import :methods.rvol_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.value_node;

export namespace pludux {

class RvolNode {
public:
  explicit RvolNode(std::size_t period = 14)
  : RvolNode{ValueNode{static_cast<double>(period)}}
  {
  }

  explicit RvolNode(ErasedNode<ErasedSeriesMethodContext> period)
  : period_{std::move(period)}
  {
  }

  auto operator==(const RvolNode& other) const noexcept -> bool = default;

  auto period(this const RvolNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.period_;
  }

  void period(this RvolNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

  void period(this RvolNode& self,
              ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  ErasedNode<ErasedSeriesMethodContext> period_;
};

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const RvolNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return ErasedSeriesMethod<TContext>{RvolMethod{period}};
}

} // namespace pludux
