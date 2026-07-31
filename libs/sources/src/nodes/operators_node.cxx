module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:nodes.operators_node;

import :node_to_erased_method;
import :methods.operators_method;
import :nodes.erased_node;

export namespace pludux {
template<typename TBinaryFn>
class BinaryOperatorNode {
public:
  BinaryOperatorNode(ErasedNode<ErasedSeriesMethodContext> operand1,
                     ErasedNode<ErasedSeriesMethodContext> operand2)
  : operand1_{std::move(operand1)}
  , operand2_{std::move(operand2)}
  {
  }

  auto operator==(const BinaryOperatorNode& other) const noexcept
   -> bool = default;

  auto operand1(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.operand1_;
  }

  void operand1(this BinaryOperatorNode& self,
                ErasedNode<ErasedSeriesMethodContext> operand1) noexcept
  {
    self.operand1_ = std::move(operand1);
  }

  auto left(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.operand1();
  }

  void left(this BinaryOperatorNode& self,
            ErasedNode<ErasedSeriesMethodContext> left) noexcept
  {
    self.operand1(std::move(left));
  }

  auto operand2(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.operand2_;
  }

  void operand2(this BinaryOperatorNode& self,
                ErasedNode<ErasedSeriesMethodContext> operand2) noexcept
  {
    self.operand2_ = std::move(operand2);
  }

  auto right(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.operand2();
  }

  void right(this BinaryOperatorNode& self,
             ErasedNode<ErasedSeriesMethodContext> right) noexcept
  {
    self.operand2(std::move(right));
  }

private:
  ErasedNode<ErasedSeriesMethodContext> operand1_;
  ErasedNode<ErasedSeriesMethodContext> operand2_;
};

template<typename TUnaryFn>
class UnaryOperatorNode {
public:
  explicit UnaryOperatorNode(ErasedNode<ErasedSeriesMethodContext> operand)
  : operand_{std::move(operand)}
  {
  }

  auto operator==(const UnaryOperatorNode& other) const noexcept
   -> bool = default;

  auto operand(this const UnaryOperatorNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.operand_;
  }

  void operand(this UnaryOperatorNode& self,
               ErasedNode<ErasedSeriesMethodContext> operand) noexcept
  {
    self.operand_ = std::move(operand);
  }

private:
  ErasedNode<ErasedSeriesMethodContext> operand_;
};

using MultiplyNode = BinaryOperatorNode<std::multiplies<>>;

using DivideNode = BinaryOperatorNode<std::divides<>>;

using AddNode = BinaryOperatorNode<std::plus<>>;

using SubtractNode = BinaryOperatorNode<std::minus<>>;

using NegateNode = UnaryOperatorNode<std::negate<>>;

using AbsNode = UnaryOperatorNode<Absolute<>>;

using AbsDiffNode = BinaryOperatorNode<AbsoluteDifference<>>;

using SqrtNode = UnaryOperatorNode<SquareRoot<>>;

using MaxNode = BinaryOperatorNode<Maximum<>>;

using MinNode = BinaryOperatorNode<Minimum<>>;

using PositivePartNode = UnaryOperatorNode<PositivePart<>>;

using NegativePartNode = UnaryOperatorNode<NegativePart<>>;

template<typename TBinaryFn, MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const BinaryOperatorNode<TBinaryFn>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  auto operand1 = node_to_erased_method<TContext>(node.operand1(), context);
  auto operand2 = node_to_erased_method<TContext>(node.operand2(), context);
  return ErasedSeriesMethod<TContext>{
   BinaryOperatorMethod<TBinaryFn,
                        ErasedSeriesMethod<TContext>,
                        ErasedSeriesMethod<TContext>>{std::move(operand1),
                                                      std::move(operand2)}};
}

template<typename TUnaryFn, MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const UnaryOperatorNode<TUnaryFn>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{
   UnaryOperatorMethod<TUnaryFn, ErasedSeriesMethod<TContext>>{
    node_to_erased_method<TContext>(node.operand(), context)}};
}

} // namespace pludux
