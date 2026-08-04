module;

#include <functional>
#include <utility>

export module pludux:nodes.logical_node;

import :methods.logical_method;
import :node_to_erased_method;
import :nodes.erased_node;

export namespace pludux {

template<typename TOperator, MethodContextable TContext>
class BinaryLogicalNode {
public:
  BinaryLogicalNode(ErasedNode<TContext> first_condition,
                    ErasedNode<TContext> second_condition)
  : first_condition_{std::move(first_condition)}
  , second_condition_{std::move(second_condition)}
  {
  }

  auto operator==(const BinaryLogicalNode& other) const noexcept
   -> bool = default;

  auto first_condition(this const BinaryLogicalNode& self)
   -> const ErasedNode<TContext>&
  {
    return self.first_condition_;
  }

  void first_condition(this BinaryLogicalNode& self,
                       ErasedNode<TContext> condition)
  {
    self.first_condition_ = std::move(condition);
  }

  auto second_condition(this const BinaryLogicalNode& self)
   -> const ErasedNode<TContext>&
  {
    return self.second_condition_;
  }

  void second_condition(this BinaryLogicalNode& self,
                        ErasedNode<TContext> condition)
  {
    self.second_condition_ = std::move(condition);
  }

private:
  ErasedNode<TContext> first_condition_;
  ErasedNode<TContext> second_condition_;
};

template<typename TOperator, MethodContextable TContext>
class UnaryLogicalNode {
public:
  explicit UnaryLogicalNode(ErasedNode<TContext> condition)
  : other_condition_{std::move(condition)}
  {
  }

  auto operator==(const UnaryLogicalNode& other) const noexcept
   -> bool = default;

  auto other_condition(this const UnaryLogicalNode& self)
   -> const ErasedNode<TContext>&
  {
    return self.other_condition_;
  }

  void other_condition(this UnaryLogicalNode& self,
                       ErasedNode<TContext> condition)
  {
    self.other_condition_ = std::move(condition);
  }

private:
  ErasedNode<TContext> other_condition_;
};

template<MethodContextable TContext>
using LogicalAndNode = BinaryLogicalNode<std::logical_and<>, TContext>;

template<MethodContextable TContext>
using LogicalOrNode = BinaryLogicalNode<std::logical_or<>, TContext>;

template<MethodContextable TContext>
using LogicalNotNode = UnaryLogicalNode<std::logical_not<>, TContext>;

template<MethodContextable TContext>
using LogicalXorNode = BinaryLogicalNode<LogicalXor<>, TContext>;

template<typename TOperator, MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const BinaryLogicalNode<TOperator, TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  auto first_condition =
   node_to_erased_method<TContext>(node.first_condition(), context);
  auto second_condition =
   node_to_erased_method<TContext>(node.second_condition(), context);
  return ErasedSeriesMethod<TContext>{
   BinaryLogicalMethod<TOperator,
                       ErasedSeriesMethod<TContext>,
                       ErasedSeriesMethod<TContext>>{
    std::move(first_condition), std::move(second_condition)}};
}

template<typename TOperator, MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const UnaryLogicalNode<TOperator, TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{
   UnaryLogicalMethod<TOperator, ErasedSeriesMethod<TContext>>{
    node_to_erased_method<TContext>(node.other_condition(), context)}};
}

} // namespace pludux
