module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <variant>

export module pludux:nodes.rvol_node;

import :methods.rvol_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.value_node;

export namespace pludux {

template<typename TContext = std::monostate>
class RvolNode {
public:
  explicit RvolNode(std::size_t period = 14)
  : RvolNode{ValueNode{static_cast<double>(period)}}
  {
  }

   explicit RvolNode(ErasedNode<TContext> period)
  : period_{std::move(period)}
  {
  }

  auto operator==(const RvolNode& other) const noexcept -> bool = default;

  auto period(this const RvolNode& self) noexcept
    -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this RvolNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

  void period(this RvolNode& self,
               ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
   ErasedNode<TContext> period_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const RvolNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return ErasedSeriesMethod<TContext>{RvolMethod{period}};
}

} // namespace pludux
