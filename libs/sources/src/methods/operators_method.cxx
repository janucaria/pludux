module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.operators_method;

export namespace pludux {

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

/*---------------------------------------------------------------*/

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

/*---------------------------------------------------------------*/

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

/*---------------------------------------------------------------*/

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

/*---------------------------------------------------------------*/

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

/*---------------------------------------------------------------*/

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

/*---------------------------------------------------------------*/

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

/*---------------------------------------------------------------*/

template<typename TBinaryFn,
         typename TLeftOperandMethod,
         typename TRightOperandMethod>
class BinaryOperatorMethod {
public:
  BinaryOperatorMethod(TLeftOperandMethod operand1,
                       TRightOperandMethod operand2)
  : operand1_{std::move(operand1)}
  , operand2_{std::move(operand2)}
  {
  }

  auto operator==(const BinaryOperatorMethod& other) const noexcept
   -> bool = default;

  auto operand1(this const BinaryOperatorMethod& self) noexcept
   -> const TLeftOperandMethod&
  {
    return self.operand1_;
  }

  void operand1(this BinaryOperatorMethod& self,
                TLeftOperandMethod operand1) noexcept
  {
    self.operand1_ = std::move(operand1);
  }

  auto left(this const BinaryOperatorMethod& self) noexcept
   -> const TLeftOperandMethod&
  {
    return self.operand1();
  }

  void left(this BinaryOperatorMethod& self, TLeftOperandMethod left) noexcept
  {
    self.operand1(std::move(left));
  }

  auto operand2(this const BinaryOperatorMethod& self) noexcept
   -> const TRightOperandMethod&
  {
    return self.operand2_;
  }

  void operand2(this BinaryOperatorMethod& self,
                TRightOperandMethod operand2) noexcept
  {
    self.operand2_ = std::move(operand2);
  }

  auto right(this const BinaryOperatorMethod& self) noexcept
   -> const TRightOperandMethod&
  {
    return self.operand2();
  }

  void right(this BinaryOperatorMethod& self,
             TRightOperandMethod right) noexcept
  {
    self.operand2(std::move(right));
  }

private:
  TLeftOperandMethod operand1_;
  TRightOperandMethod operand2_;
};

template<typename TUnaryFn, typename TOperandMethod>
class UnaryOperatorMethod {
public:
  explicit UnaryOperatorMethod(TOperandMethod operand)
  : operand_{std::move(operand)}
  {
  }

  auto operator==(const UnaryOperatorMethod& other) const noexcept
   -> bool = default;

  auto operand(this const UnaryOperatorMethod& self) noexcept
   -> const TOperandMethod&
  {
    return self.operand_;
  }

  void operand(this UnaryOperatorMethod& self, TOperandMethod operand) noexcept
  {
    self.operand_ = std::move(operand);
  }

private:
  TOperandMethod operand_;
};

template<typename TLeftOperandMethod, typename TRightOperandMethod>
using MultiplyMethod = BinaryOperatorMethod<std::multiplies<>,
                                            TLeftOperandMethod,
                                            TRightOperandMethod>;

template<typename TLeftOperandMethod, typename TRightOperandMethod>
using DivideMethod =
 BinaryOperatorMethod<std::divides<>, TLeftOperandMethod, TRightOperandMethod>;

template<typename TLeftOperandMethod, typename TRightOperandMethod>
using AddMethod =
 BinaryOperatorMethod<std::plus<>, TLeftOperandMethod, TRightOperandMethod>;

template<typename TLeftOperandMethod, typename TRightOperandMethod>
using SubtractMethod =
 BinaryOperatorMethod<std::minus<>, TLeftOperandMethod, TRightOperandMethod>;

template<typename TOperandMethod>
using NegateMethod = UnaryOperatorMethod<std::negate<>, TOperandMethod>;

template<typename TOperandMethod>
using AbsMethod = UnaryOperatorMethod<Absolute<>, TOperandMethod>;

template<typename TLeftOperandMethod, typename TRightOperandMethod>
using AbsDiffMethod = BinaryOperatorMethod<AbsoluteDifference<>,
                                           TLeftOperandMethod,
                                           TRightOperandMethod>;

template<typename TOperandMethod>
using SqrtMethod = UnaryOperatorMethod<SquareRoot<>, TOperandMethod>;

template<typename TLeftOperandMethod, typename TRightOperandMethod>
using MaxMethod =
 BinaryOperatorMethod<Maximum<>, TLeftOperandMethod, TRightOperandMethod>;

template<typename TLeftOperandMethod, typename TRightOperandMethod>
using MinMethod =
 BinaryOperatorMethod<Minimum<>, TLeftOperandMethod, TRightOperandMethod>;

template<typename TOperandMethod>
using PositivePartMethod = UnaryOperatorMethod<PositivePart<>, TOperandMethod>;

template<typename TOperandMethod>
using NegativePartMethod = UnaryOperatorMethod<NegativePart<>, TOperandMethod>;

} // namespace pludux