module;

#include <cmath>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

export module pludux.screener.arithmetic_method;

import pludux.asset_snapshot;
import pludux.series;
import pludux.screener.screener_method;

namespace pludux::screener {

namespace details {

template<typename, typename T>
class BinaryArithmeticMethod {
public:
  BinaryArithmeticMethod(ScreenerMethod operand1,
                         ScreenerMethod operand2,
                         std::size_t offset = 0)
  : operand1_{std::move(operand1)}
  , operand2_{std::move(operand2)}
  , offset_{offset}
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    const auto operand1_series = operand1_(asset_data);
    const auto operand2_series = operand2_(asset_data);

    auto result = BinaryFnSeries<T,
                                 std::decay_t<decltype(operand1_series)>,
                                 std::decay_t<decltype(operand2_series)>>{
     operand1_series, operand2_series};

    return SubSeries{PolySeries<double>{result}, 0};
  }

  auto operator==(const BinaryArithmeticMethod& other) const noexcept
   -> bool = default;

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

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
  std::size_t offset_;
};

template<typename T>
class UnaryArithmeticMethod {
public:
  explicit UnaryArithmeticMethod(ScreenerMethod operand, std::size_t offset = 0)
  : operand_{std::move(operand)}
  , offset_{offset}
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    const auto operand_series = operand_(asset_data);

    auto result =
     UnaryFnSeries<T, std::decay_t<decltype(operand_series)>>{operand_series};

    return SubSeries{PolySeries<double>{result}, 0};
  }

  auto operator==(const UnaryArithmeticMethod& other) const noexcept
   -> bool = default;

  auto operand() const noexcept -> const ScreenerMethod&
  {
    return operand_;
  }

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

private:
  ScreenerMethod operand_;
  std::size_t offset_;
};

} // namespace details

export class MultiplyMethod
: public details::BinaryArithmeticMethod<MultiplyMethod, std::multiplies<>> {
public:
  // using details::BinaryArithmeticMethod<MultiplyMethod, std::multiplies<>>::
  //  BinaryArithmeticMethod;

  MultiplyMethod(ScreenerMethod multiplicand,
                 ScreenerMethod multiplier,
                 std::size_t offset = 0)
  : BinaryArithmeticMethod{
     std::move(multiplicand), std::move(multiplier), offset}
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
: public details::BinaryArithmeticMethod<DivideMethod, std::divides<>> {
public:
  // using details::BinaryArithmeticMethod<DivideMethod,
  //                                       std::divides<>>::BinaryArithmeticMethod;

  DivideMethod(ScreenerMethod dividend,
               ScreenerMethod divisor,
               std::size_t offset = 0)
  : BinaryArithmeticMethod{std::move(dividend), std::move(divisor), offset}
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

export class AddMethod
: public details::BinaryArithmeticMethod<AddMethod, std::plus<>> {
public:
  // using details::BinaryArithmeticMethod<AddMethod,
  //                                       std::plus<>>::BinaryArithmeticMethod;

  AddMethod(ScreenerMethod augend,
            ScreenerMethod addend,
            std::size_t offset = 0)
  : BinaryArithmeticMethod{std::move(augend), std::move(addend), offset}
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
: public details::BinaryArithmeticMethod<SubtractMethod, std::minus<>> {
public:
  // using details::BinaryArithmeticMethod<SubtractMethod,
  //                                       std::minus<>>::BinaryArithmeticMethod;

  SubtractMethod(ScreenerMethod minuend,
                 ScreenerMethod subtrahend,
                 std::size_t offset = 0)
  : BinaryArithmeticMethod{std::move(minuend), std::move(subtrahend), offset}
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

export class NegateMethod
: public details::UnaryArithmeticMethod<std::negate<>> {
public:
  // using details::UnaryArithmeticMethod<std::negate<>>::UnaryArithmeticMethod;

  NegateMethod(ScreenerMethod operand, std::size_t offset = 0)
  : UnaryArithmeticMethod{std::move(operand), offset}
  {
  }
};

namespace details {
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
} // namespace details

export class PercentageMethod
: public details::BinaryArithmeticMethod<PercentageMethod,
                                         details::Percentages<>> {
public:
  // using details::BinaryArithmeticMethod<
  //  PercentageMethod,
  //  details::Percentages<>>::BinaryArithmeticMethod;

  PercentageMethod(ScreenerMethod total,
                   ScreenerMethod percent,
                   std::size_t offset = 0)
  : BinaryArithmeticMethod{std::move(total), std::move(percent), offset}
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

namespace details {
template<typename T = void>
struct AbsoluteDifference {
  auto operator()(T left, T right) const -> T
  {
    return std::abs(left - right);
  }
};

template<>
struct AbsoluteDifference<void> {
  auto operator()(auto left, auto right) const
  {
    return std::abs(left - right);
  }
};
} // namespace details

export class AbsDiffMethod
: public details::BinaryArithmeticMethod<AbsDiffMethod,
                                         details::AbsoluteDifference<>> {
public:
  // using details::BinaryArithmeticMethod<
  //  AbsDiffMethod,
  //  details::AbsoluteDifference<>>::BinaryArithmeticMethod;

  AbsDiffMethod(ScreenerMethod left,
                ScreenerMethod right,
                std::size_t offset = 0)
  : BinaryArithmeticMethod{std::move(left), std::move(right), offset}
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
