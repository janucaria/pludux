export module pludux:nodes.boolean_node;

import :methods.boolean_method;
import :node_to_erased_method;

export namespace pludux {

template<bool boolean_value>
struct BooleanNode {
  auto operator==(const BooleanNode&) const noexcept -> bool = default;
};

using TrueNode = BooleanNode<true>;
using FalseNode = BooleanNode<false>;

template<bool boolean_value, MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const BooleanNode<boolean_value>&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{BooleanMethod<boolean_value>{}};
}

} // namespace pludux
