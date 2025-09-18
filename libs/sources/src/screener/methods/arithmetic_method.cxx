module;

#include <cmath>
#include <cstddef>
#include <functional>
#include <utility>

export module pludux.screener.arithmetic_method;

import pludux.asset_snapshot;
import pludux.screener.screener_method;

namespace pludux::screener {

template<typename, typename T>
class BinaryArithmeticMethod {
public:
  BinaryArithmeticMethod(ScreenerMethod operand1, ScreenerMethod operand2)
  : operand1_{std::move(operand1)}
  , operand2_{std::move(operand2)}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    const auto operand1_series = operand1_(asset_data);
    const auto operand2_series = operand2_(asset_data);

    auto result = BinaryOpSeries<T,
                                 std::decay_t<decltype(operand1_series)>,
                                 std::decay_t<decltype(operand2_series)>>{
     operand1_series, operand2_series};

    return result;
  }

  auto operator==(const BinaryArithmeticMethod& other) const noexcept
   -> bool = default;

  auto operand1() const noexcept -> const ScreenerMethod&
  {
    return operand1_;
  }

  auto operand2() const noexcept -> const ScreenerMethod&
  {
    return operand2_;
  }

private:
  ScreenerMethod operand1_;
  ScreenerMethod operand2_;
};

template<typename T>
class UnaryArithmeticMethod {
public:
  explicit UnaryArithmeticMethod(ScreenerMethod operand)
  : operand_{std::move(operand)}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    const auto operand_series = operand_(asset_data);

    auto result =
     UnaryOpSeries<T, std::decay_t<decltype(operand_series)>>{operand_series};

    return result;
  }

  auto operator==(const UnaryArithmeticMethod& other) const noexcept
   -> bool = default;

  auto operand() const noexcept -> const ScreenerMethod&
  {
    return operand_;
  }

private:
  ScreenerMethod operand_;
};

export class MultiplyMethod
: public BinaryArithmeticMethod<MultiplyMethod, std::multiplies<>> {
public:
  MultiplyMethod(ScreenerMethod multiplicand, ScreenerMethod multiplier)
  : BinaryArithmeticMethod<MultiplyMethod, std::multiplies<>>{
     std::move(multiplicand), std::move(multiplier)}
  {
  }

  auto multiplicand() const noexcept -> const ScreenerMethod&
  {
    return operand1();
  }

  auto multiplier() const noexcept -> const ScreenerMethod&
  {
    return operand2();
  }
};

export class DivideMethod
: public BinaryArithmeticMethod<DivideMethod, std::divides<>> {
public:
  DivideMethod(ScreenerMethod dividend, ScreenerMethod divisor)
  : BinaryArithmeticMethod<DivideMethod, std::divides<>>{std::move(dividend),
                                                         std::move(divisor)}
  {
  }

  auto dividend() const noexcept -> const ScreenerMethod&
  {
    return operand1();
  }

  auto divisor() const noexcept -> const ScreenerMethod&
  {
    return operand2();
  }
};

export class AddMethod : public BinaryArithmeticMethod<AddMethod, std::plus<>> {
public:
  AddMethod(ScreenerMethod augend, ScreenerMethod addend)
  : BinaryArithmeticMethod<AddMethod, std::plus<>>{std::move(augend),
                                                   std::move(addend)}
  {
  }

  auto augend() const noexcept -> const ScreenerMethod&
  {
    return operand1();
  }

  auto addend() const noexcept -> const ScreenerMethod&
  {
    return operand2();
  }
};

export class SubtractMethod
: public BinaryArithmeticMethod<SubtractMethod, std::minus<>> {
public:
  SubtractMethod(ScreenerMethod minuend, ScreenerMethod subtrahend)
  : BinaryArithmeticMethod<SubtractMethod, std::minus<>>{std::move(minuend),
                                                         std::move(subtrahend)}
  {
  }

  auto minuend() const noexcept -> const ScreenerMethod&
  {
    return operand1();
  }

  auto subtrahend() const noexcept -> const ScreenerMethod&
  {
    return operand2();
  }
};

export class NegateMethod : public UnaryArithmeticMethod<std::negate<>> {
public:
  explicit NegateMethod(ScreenerMethod operand)
  : UnaryArithmeticMethod<std::negate<>>(std::move(operand))
  {
  }
};

template<typename T = void>
struct Percentages {
  auto operator()(T total, T percent) const -> T
  {
    return total * (percent / 100.0);
  }
};

template<>
struct Percentages<void> {
  auto operator()(auto total, auto percent) const
  {
    return total * (percent / 100.0);
  }
};

export class PercentageMethod
: public BinaryArithmeticMethod<PercentageMethod, Percentages<>> {
public:
  PercentageMethod(ScreenerMethod total, ScreenerMethod percent)
  : BinaryArithmeticMethod<PercentageMethod, Percentages<>>{std::move(total),
                                                            std::move(percent)}
  {
  }

  auto total() const noexcept -> const ScreenerMethod&
  {
    return operand1();
  }

  auto percent() const noexcept -> const ScreenerMethod&
  {
    return operand2();
  }
};

template<typename T = void>
struct AbsoluteDifference {
  auto operator()(this const auto, T left, T right) -> T
  {
    return std::abs(left - right);
  }
};

template<>
struct AbsoluteDifference<void> {
  auto operator()(this const auto, auto left, auto right)
  {
    return std::abs(left - right);
  }
};

export class AbsDiffMethod
: public BinaryArithmeticMethod<AbsDiffMethod, AbsoluteDifference<>> {
public:
  AbsDiffMethod(ScreenerMethod left, ScreenerMethod right)
  : BinaryArithmeticMethod<AbsDiffMethod, AbsoluteDifference<>>{
     std::move(left), std::move(right)}
  {
  }

  auto minuend() const noexcept -> const ScreenerMethod&
  {
    return operand1();
  }

  auto subtrahend() const noexcept -> const ScreenerMethod&
  {
    return operand2();
  }
};

} // namespace pludux::screener