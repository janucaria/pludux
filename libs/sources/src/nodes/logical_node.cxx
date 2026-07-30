module;

#include <functional>
#include <utility>

export module pludux:nodes.logical_node;

import :methods.logical_method;
import :node_to_erased_method;
import :nodes.erased_node;

export namespace pludux {

template<typename TOperator>
class BinaryLogicalNode {
public:
  BinaryLogicalNode(ErasedNode first_condition, ErasedNode second_condition)
  : first_condition_{std::move(first_condition)}
  , second_condition_{std::move(second_condition)}
  {
  }

  auto operator==(const BinaryLogicalNode& other) const noexcept
   -> bool = default;

  auto first_condition(this const BinaryLogicalNode& self) -> const ErasedNode&
  {
    return self.first_condition_;
  }

  void first_condition(this BinaryLogicalNode& self, ErasedNode condition)
  {
    self.first_condition_ = std::move(condition);
  }

  auto second_condition(this const BinaryLogicalNode& self) -> const ErasedNode&
  {
    return self.second_condition_;
  }

  void second_condition(this BinaryLogicalNode& self, ErasedNode condition)
  {
    self.second_condition_ = std::move(condition);
  }

private:
  ErasedNode first_condition_;
  ErasedNode second_condition_;
};

template<typename TOperator>
class UnaryLogicalNode {
public:
  explicit UnaryLogicalNode(ErasedNode condition)
  : other_condition_{std::move(condition)}
  {
  }

  auto operator==(const UnaryLogicalNode& other) const noexcept
   -> bool = default;

  auto other_condition(this const UnaryLogicalNode& self) -> const ErasedNode&
  {
    return self.other_condition_;
  }

  void other_condition(this UnaryLogicalNode& self, ErasedNode condition)
  {
    self.other_condition_ = std::move(condition);
  }

private:
  ErasedNode other_condition_;
};

using LogicalAndNode = BinaryLogicalNode<std::logical_and<>>;

using LogicalOrNode = BinaryLogicalNode<std::logical_or<>>;

using LogicalNotNode = UnaryLogicalNode<std::logical_not<>>;

using LogicalXorNode = BinaryLogicalNode<LogicalXor<>>;

template<typename TOperator>
auto pludux_tag_invoke(NodeToErasedMethod,
                       const BinaryLogicalNode<TOperator>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  auto first_condition = node_to_erased_method(node.first_condition(), context);
  auto second_condition =
   node_to_erased_method(node.second_condition(), context);
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   BinaryLogicalMethod<TOperator,
                       ErasedSeriesMethod<ErasedSeriesMethodContext>,
                       ErasedSeriesMethod<ErasedSeriesMethodContext>>{
    std::move(first_condition), std::move(second_condition)}};
}

template<typename TOperator>
auto pludux_tag_invoke(NodeToErasedMethod,
                       const UnaryLogicalNode<TOperator>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   UnaryLogicalMethod<TOperator, ErasedSeriesMethod<ErasedSeriesMethodContext>>{
    node_to_erased_method(node.other_condition(), context)}};
}

} // namespace pludux
