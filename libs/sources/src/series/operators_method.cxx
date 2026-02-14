module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:series.operators_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

namespace pludux {

template<template<typename, typename> typename,
         typename TBinaryFn,
         typename TMethodOp1,
         typename TMethodOp2>
class BinaryOperatorMethod {
public:
  using ResultType = std::common_type_t<typename TMethodOp1::ResultType,
                                        typename TMethodOp2::ResultType>;

  BinaryOperatorMethod(TMethodOp1 operand1, TMethodOp2 operand2)
  : operand1_{std::move(operand1)}
  , operand2_{std::move(operand2)}
  {
  }

  auto operator==(const BinaryOperatorMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const BinaryOperatorMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    const auto operand1_result = self.operand1_(asset_snapshot, context);
    const auto operand2_result = self.operand2_(asset_snapshot, context);
    const auto result = TBinaryFn{}(operand1_result, operand2_result);
    return result;
  }

  auto operator()(this const BinaryOperatorMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto operand1(this const BinaryOperatorMethod& self) noexcept
   -> const TMethodOp1&
  {
    return self.operand1_;
  }

  void operand1(this BinaryOperatorMethod& self, TMethodOp1 operand1) noexcept
  {
    self.operand1_ = std::move(operand1);
  }

  auto left(this const BinaryOperatorMethod& self) noexcept -> const TMethodOp1&
  {
    return self.operand1();
  }

  void left(this BinaryOperatorMethod& self, TMethodOp1 left) noexcept
  {
    self.operand1(std::move(left));
  }

  auto operand2(this const BinaryOperatorMethod& self) noexcept
   -> const TMethodOp2&
  {
    return self.operand2_;
  }

  void operand2(this BinaryOperatorMethod& self, TMethodOp2 operand2) noexcept
  {
    self.operand2_ = std::move(operand2);
  }

  auto right(this const BinaryOperatorMethod& self) noexcept
   -> const TMethodOp2&
  {
    return self.operand2();
  }

  void right(this BinaryOperatorMethod& self, TMethodOp2 right) noexcept
  {
    self.operand2(std::move(right));
  }

private:
  TMethodOp1 operand1_;
  TMethodOp2 operand2_;
};

template<template<typename> typename, typename TUnaryFn, typename TMethodOp>
class UnaryOperatorMethod {
public:
  using ResultType = typename TMethodOp::ResultType;

  explicit UnaryOperatorMethod(TMethodOp operand)
  : operand_{std::move(operand)}
  {
  }

  auto operator==(const UnaryOperatorMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const UnaryOperatorMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    const auto operand_result = self.operand_(asset_snapshot, context);
    return TUnaryFn{}(operand_result);
  }

  auto operator()(this const UnaryOperatorMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto operand(this const UnaryOperatorMethod& self) noexcept
   -> const TMethodOp&
  {
    return self.operand_;
  }

  void operand(this UnaryOperatorMethod& self, TMethodOp operand) noexcept
  {
    self.operand_ = std::move(operand);
  }

private:
  TMethodOp operand_;
};

export template<typename TMethodOp1, typename TMethodOp2>
class MultiplyMethod
: public BinaryOperatorMethod<MultiplyMethod,
                              std::multiplies<>,
                              TMethodOp1,
                              TMethodOp2> {
public:
  MultiplyMethod(TMethodOp1 multiplicand, TMethodOp2 multiplier)
  : BinaryOperatorMethod<MultiplyMethod,
                         std::multiplies<>,
                         TMethodOp1,
                         TMethodOp2>{std::move(multiplicand),
                                     std::move(multiplier)}
  {
  }

  auto multiplicand(this const MultiplyMethod& self) noexcept
   -> const TMethodOp1&
  {
    return self.operand1();
  }

  auto multiplier(this const MultiplyMethod& self) noexcept -> const TMethodOp2&
  {
    return self.operand2();
  }
};

export template<typename TMethodOp1, typename TMethodOp2>
class DivideMethod
: public BinaryOperatorMethod<DivideMethod,
                              std::divides<>,
                              TMethodOp1,
                              TMethodOp2> {
public:
  DivideMethod(TMethodOp1 dividend, TMethodOp2 divisor)
  : BinaryOperatorMethod<DivideMethod, std::divides<>, TMethodOp1, TMethodOp2>{
     std::move(dividend), std::move(divisor)}
  {
  }

  auto dividend(this const DivideMethod& self) noexcept -> const TMethodOp1&
  {
    return self.operand1();
  }

  auto divisor(this const DivideMethod& self) noexcept -> const TMethodOp2&
  {
    return self.operand2();
  }
};

export template<typename TMethodOp1, typename TMethodOp2>
class AddMethod
: public BinaryOperatorMethod<AddMethod, std::plus<>, TMethodOp1, TMethodOp2> {
public:
  AddMethod(TMethodOp1 augend, TMethodOp2 addend)
  : BinaryOperatorMethod<AddMethod, std::plus<>, TMethodOp1, TMethodOp2>{
     std::move(augend), std::move(addend)}
  {
  }

  auto augend(this const AddMethod& self) noexcept -> const TMethodOp1&
  {
    return self.operand1();
  }

  auto addend(this const AddMethod& self) noexcept -> const TMethodOp2&
  {
    return self.operand2();
  }
};

export template<typename TMethodOp1, typename TMethodOp2>
class SubtractMethod
: public BinaryOperatorMethod<SubtractMethod,
                              std::minus<>,
                              TMethodOp1,
                              TMethodOp2> {
public:
  SubtractMethod(TMethodOp1 minuend, TMethodOp2 subtrahend)
  : BinaryOperatorMethod<SubtractMethod, std::minus<>, TMethodOp1, TMethodOp2>{
     std::move(minuend), std::move(subtrahend)}
  {
  }

  auto minuend(this const SubtractMethod& self) noexcept -> const TMethodOp1&
  {
    return self.operand1();
  }

  auto subtrahend(this const SubtractMethod& self) noexcept -> const TMethodOp2&
  {
    return self.operand2();
  }
};

export template<typename TMethodOp>
class NegateMethod
: public UnaryOperatorMethod<NegateMethod, std::negate<>, TMethodOp> {
public:
  explicit NegateMethod(TMethodOp operand)
  : UnaryOperatorMethod<NegateMethod, std::negate<>, TMethodOp>{
     std::move(operand)}
  {
  }
};

// -----------------------------------------------------------------------

template<typename T = void>
struct Absolute {
  auto operator()(this const Absolute, T value) -> T
  {
    return std::abs(value);
  }
};

template<>
struct Absolute<void> {
  auto operator()(this const Absolute, auto value)
  {
    return std::abs(value);
  }
};

export template<typename TMethodOp>
class AbsMethod : public UnaryOperatorMethod<AbsMethod, Absolute<>, TMethodOp> {
public:
  explicit AbsMethod(TMethodOp operand)
  : UnaryOperatorMethod<AbsMethod, Absolute<>, TMethodOp>{std::move(operand)}
  {
  }
};

// -----------------------------------------------------------------------

template<typename T = void>
struct AbsoluteDifference {
  auto operator()(this const AbsoluteDifference, T left, T right) -> T
  {
    return std::abs(left - right);
  }
};

template<>
struct AbsoluteDifference<void> {
  auto operator()(this const AbsoluteDifference, auto left, auto right)
  {
    return std::abs(left - right);
  }
};

export template<typename TMethodOp1, typename TMethodOp2>
class AbsDiffMethod
: public BinaryOperatorMethod<AbsDiffMethod,
                              AbsoluteDifference<>,
                              TMethodOp1,
                              TMethodOp2> {
public:
  AbsDiffMethod(TMethodOp1 left, TMethodOp2 right)
  : BinaryOperatorMethod<AbsDiffMethod,
                         AbsoluteDifference<>,
                         TMethodOp1,
                         TMethodOp2>{std::move(left), std::move(right)}
  {
  }

  auto minuend(this const AbsDiffMethod& self) noexcept -> const TMethodOp1&
  {
    return self.operand1();
  }

  auto subtrahend(this const AbsDiffMethod& self) noexcept -> const TMethodOp2&
  {
    return self.operand2();
  }
};

// -----------------------------------------------------------------------
template<typename T = void>
struct SquareRoot {
  auto operator()(this const SquareRoot, T value) -> T
  {
    return std::sqrt(value);
  }
};

template<>
struct SquareRoot<void> {
  auto operator()(this const SquareRoot, auto value)
  {
    return std::sqrt(value);
  }
};

export template<typename TMethodOp>
class SqrtMethod
: public UnaryOperatorMethod<SqrtMethod, SquareRoot<>, TMethodOp> {
public:
  explicit SqrtMethod(TMethodOp operand)
  : UnaryOperatorMethod<SqrtMethod, SquareRoot<>, TMethodOp>{std::move(operand)}
  {
  }
};

// -----------------------------------------------------------------------

template<typename T = void>
struct Maximum {
  auto operator()(this const Maximum, T left, T right) -> T
  {
    return std::max(left, right);
  }
};

template<>
struct Maximum<void> {
  auto operator()(this const Maximum, auto left, auto right)
  {
    return std::max(left, right);
  }
};

export template<typename TMethodOp1, typename TMethodOp2>
class MaxMethod
: public BinaryOperatorMethod<MaxMethod, Maximum<>, TMethodOp1, TMethodOp2> {
public:
  MaxMethod(TMethodOp1 left, TMethodOp2 right)
  : BinaryOperatorMethod<MaxMethod, Maximum<>, TMethodOp1, TMethodOp2>{
     std::move(left), std::move(right)}
  {
  }

  auto left(this const MaxMethod& self) noexcept -> const TMethodOp1&
  {
    return self.operand1();
  }

  auto right(this const MaxMethod& self) noexcept -> const TMethodOp2&
  {
    return self.operand2();
  }
};

// -----------------------------------------------------------------------

template<typename T = void>
struct Minimum {
  auto operator()(this const Minimum, T left, T right) -> T
  {
    return std::min(left, right);
  }
};

template<>
struct Minimum<void> {
  auto operator()(this const Minimum, auto left, auto right)
  {
    return std::min(left, right);
  }
};

export template<typename TMethodOp1, typename TMethodOp2>
class MinMethod
: public BinaryOperatorMethod<MinMethod, Minimum<>, TMethodOp1, TMethodOp2> {
public:
  MinMethod(TMethodOp1 left, TMethodOp2 right)
  : BinaryOperatorMethod<MinMethod, Minimum<>, TMethodOp1, TMethodOp2>{
     std::move(left), std::move(right)}
  {
  }

  auto left(this const MinMethod& self) noexcept -> const TMethodOp1&
  {
    return self.operand1();
  }

  auto right(this const MinMethod& self) noexcept -> const TMethodOp2&
  {
    return self.operand2();
  }
};

// -----------------------------------------------------------------------

template<typename T = void>
struct PositivePart {
  auto operator()(this const PositivePart, T value) -> T
  {
    if(std::isnan(value)) {
      return std::numeric_limits<T>::quiet_NaN();
    }
    return value > 0 ? value : T{0};
  }
};

template<>
struct PositivePart<void> {
  auto operator()(this const PositivePart, auto value)
  {
    using TValue = decltype(value);

    if(std::isnan(value)) {
      return std::numeric_limits<TValue>::quiet_NaN();
    }

    return value > 0 ? value : TValue{0};
  }
};

export template<typename TMethodOp>
class PositivePartMethod
: public UnaryOperatorMethod<PositivePartMethod, PositivePart<>, TMethodOp> {
public:
  explicit PositivePartMethod(TMethodOp operand)
  : UnaryOperatorMethod<PositivePartMethod, PositivePart<>, TMethodOp>{
     std::move(operand)}
  {
  }
};

// -----------------------------------------------------------------------

template<typename T = void>
struct NegativePart {
  auto operator()(this const NegativePart, T value) -> T
  {
    if(std::isnan(value)) {
      return std::numeric_limits<T>::quiet_NaN();
    }
    return value < 0 ? -value : T{0};
  }
};

template<>
struct NegativePart<void> {
  auto operator()(this const NegativePart, auto value)
  {
    if(std::isnan(value)) {
      return std::numeric_limits<decltype(value)>::quiet_NaN();
    }
    return value < 0 ? -value : decltype(value){0};
  }
};

export template<typename TMethodOp>
class NegativePartMethod
: public UnaryOperatorMethod<NegativePartMethod, NegativePart<>, TMethodOp> {
public:
  explicit NegativePartMethod(TMethodOp operand)
  : UnaryOperatorMethod<NegativePartMethod, NegativePart<>, TMethodOp>{
     std::move(operand)}
  {
  }
};

} // namespace pludux