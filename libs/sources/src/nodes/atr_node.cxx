module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <variant>

export module pludux:nodes.atr_node;

import :ma_node_type;
import :methods.atr_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.value_node;

export namespace pludux {

template<typename TContext = std::monostate>
class AtrNode {
public:
  AtrNode()
  : AtrNode{14}
  {
  }

  explicit AtrNode(std::size_t period,
                   MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : AtrNode{ValueNode{static_cast<double>(period)}, ma_smoothing_type}
  {
  }

   explicit AtrNode(ErasedNode<TContext> period,
                   MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : period_{std::move(period)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const AtrNode& other) const noexcept -> bool = default;

  auto period(this const AtrNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this AtrNode& self, std::size_t new_period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(new_period)};
  }

  void period(this AtrNode& self,
               ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  auto ma_smoothing_type(this const AtrNode& self) noexcept -> MaNodeType
  {
    return self.ma_smoothing_type_;
  }

  void ma_smoothing_type(this AtrNode& self, MaNodeType new_type) noexcept
  {
    self.ma_smoothing_type_ = new_type;
  }

private:
   ErasedNode<TContext> period_;
  MaNodeType ma_smoothing_type_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const AtrNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto period = node_to_erased_method<TContext>(node.period(), context);

  return ErasedSeriesMethod<TContext>{
   AtrMethod{period, static_cast<MaMethodType>(node.ma_smoothing_type())}};
}

} // namespace pludux
