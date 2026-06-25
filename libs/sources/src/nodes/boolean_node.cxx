export module pludux:nodes.boolean_node;


export namespace pludux {

template<bool boolean_value>
struct BooleanNode {
  auto operator==(const BooleanNode&) const noexcept -> bool = default;

};

using TrueNode = BooleanNode<true>;
using FalseNode = BooleanNode<false>;

} // namespace pludux