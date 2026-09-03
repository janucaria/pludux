module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#include <variant>

export module pludux:nodes.operators_node;

import :node_to_erased_method;
import :methods.operators_method;
import :nodes.erased_node;

export namespace pludux {
template<typename TBinaryFn, typename TContext = std::monostate>
class BinaryOperatorNode {
public:
  BinaryOperatorNode(ErasedNode<TContext> operand1,
                     ErasedNode<TContext> operand2)
  : operand1_{std::move(operand1)}
  , operand2_{std::move(operand2)}
  {
  }

  auto operator==(const BinaryOperatorNode& other) const noexcept
   -> bool = default;

  auto operand1(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.operand1_;
  }

  void operand1(this BinaryOperatorNode& self,
                 ErasedNode<TContext> operand1) noexcept
  {
    self.operand1_ = std::move(operand1);
  }

  auto left(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.operand1();
  }

  void left(this BinaryOperatorNode& self,
             ErasedNode<TContext> left) noexcept
  {
    self.operand1(std::move(left));
  }

  auto operand2(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.operand2_;
  }

  void operand2(this BinaryOperatorNode& self,
                 ErasedNode<TContext> operand2) noexcept
  {
    self.operand2_ = std::move(operand2);
  }

  auto right(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.operand2();
  }

  void right(this BinaryOperatorNode& self,
              ErasedNode<TContext> right) noexcept
  {
    self.operand2(std::move(right));
  }

private:
  ErasedNode<TContext> operand1_;
  ErasedNode<TContext> operand2_;
};

template<typename TUnaryFn, typename TContext = std::monostate>
class UnaryOperatorNode {
public:
  explicit UnaryOperatorNode(ErasedNode<TContext> operand)
  : operand_{std::move(operand)}
  {
  }

  auto operator==(const UnaryOperatorNode& other) const noexcept
   -> bool = default;

  auto operand(this const UnaryOperatorNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.operand_;
  }

  void operand(this UnaryOperatorNode& self,
                ErasedNode<TContext> operand) noexcept
  {
    self.operand_ = std::move(operand);
  }

private:
  ErasedNode<TContext> operand_;
};

template<typename TContext = std::monostate>
using MultiplyNode = BinaryOperatorNode<std::multiplies<>, TContext>;

template<typename TContext = std::monostate>
using DivideNode = BinaryOperatorNode<std::divides<>, TContext>;

template<typename TContext = std::monostate>
using AddNode = BinaryOperatorNode<std::plus<>, TContext>;

template<typename TContext = std::monostate>
using SubtractNode = BinaryOperatorNode<std::minus<>, TContext>;

template<typename TContext = std::monostate>
using NegateNode = UnaryOperatorNode<std::negate<>, TContext>;

template<typename TContext = std::monostate>
using AbsNode = UnaryOperatorNode<Absolute<>, TContext>;

template<typename TContext = std::monostate>
using AbsDiffNode = BinaryOperatorNode<AbsoluteDifference<>, TContext>;

template<typename TContext = std::monostate>
using SqrtNode = UnaryOperatorNode<SquareRoot<>, TContext>;

template<typename TContext = std::monostate>
using MaxNode = BinaryOperatorNode<Maximum<>, TContext>;

template<typename TContext = std::monostate>
using MinNode = BinaryOperatorNode<Minimum<>, TContext>;

template<typename TContext = std::monostate>
using PositivePartNode = UnaryOperatorNode<PositivePart<>, TContext>;

template<typename TContext = std::monostate>
using NegativePartNode = UnaryOperatorNode<NegativePart<>, TContext>;

template<typename TBinaryFn, typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const BinaryOperatorNode<TBinaryFn, TContext>& node,
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

template<typename TUnaryFn, typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const UnaryOperatorNode<TUnaryFn, TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{
   UnaryOperatorMethod<TUnaryFn, ErasedSeriesMethod<TContext>>{
    node_to_erased_method<TContext>(node.operand(), context)}};
}

} // namespace pludux
