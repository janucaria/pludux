module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:nodes.operators_node;

import :methods.operators_method;
import :nodes.erased_node;

export namespace pludux {
template<typename TBinaryFn>
class BinaryOperatorNode {
public:
  BinaryOperatorNode(ErasedNode operand1, ErasedNode operand2)
  : operand1_{std::move(operand1)}
  , operand2_{std::move(operand2)}
  {
  }

  auto operator==(const BinaryOperatorNode& other) const noexcept
   -> bool = default;


  auto operand1(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode&
  {
    return self.operand1_;
  }

  void operand1(this BinaryOperatorNode& self,
                ErasedNode operand1) noexcept
  {
    self.operand1_ = std::move(operand1);
  }

  auto left(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode&
  {
    return self.operand1();
  }

  void left(this BinaryOperatorNode& self, ErasedNode left) noexcept
  {
    self.operand1(std::move(left));
  }

  auto operand2(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode&
  {
    return self.operand2_;
  }

  void operand2(this BinaryOperatorNode& self,
                ErasedNode operand2) noexcept
  {
    self.operand2_ = std::move(operand2);
  }

  auto right(this const BinaryOperatorNode& self) noexcept
   -> const ErasedNode&
  {
    return self.operand2();
  }

  void right(this BinaryOperatorNode& self, ErasedNode right) noexcept
  {
    self.operand2(std::move(right));
  }

private:
  ErasedNode operand1_;
  ErasedNode operand2_;
};

template<typename TUnaryFn>
class UnaryOperatorNode {
public:
  explicit UnaryOperatorNode(ErasedNode operand)
  : operand_{std::move(operand)}
  {
  }

  auto operator==(const UnaryOperatorNode& other) const noexcept
   -> bool = default;


  auto operand(this const UnaryOperatorNode& self) noexcept
   -> const ErasedNode&
  {
    return self.operand_;
  }

  void operand(this UnaryOperatorNode& self, ErasedNode operand) noexcept
  {
    self.operand_ = std::move(operand);
  }

private:
  ErasedNode operand_;
};

using MultiplyNode =
 BinaryOperatorNode<std::multiplies<>>;

using DivideNode =
 BinaryOperatorNode<std::divides<>>;

using AddNode =
 BinaryOperatorNode<std::plus<>>;

using SubtractNode =
 BinaryOperatorNode<std::minus<>>;

using NegateNode = UnaryOperatorNode<std::negate<>>;

using AbsNode = UnaryOperatorNode<Absolute<>>;

using AbsDiffNode =
 BinaryOperatorNode<AbsoluteDifference<>>;

using SqrtNode = UnaryOperatorNode<SquareRoot<>>;

using MaxNode =
 BinaryOperatorNode<Maximum<>>;

using MinNode =
 BinaryOperatorNode<Minimum<>>;

using PositivePartNode = UnaryOperatorNode<PositivePart<>>;

using NegativePartNode = UnaryOperatorNode<NegativePart<>>;

} // namespace pludux
