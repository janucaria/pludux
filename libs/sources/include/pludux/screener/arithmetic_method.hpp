#ifndef PLUDUX_PLUDUX_SCREENER_Arithmetic_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_Arithmetic_METHOD_HPP

#include <cstddef>

#include <pludux/screener/screener_method.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

namespace details {

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

    auto result = BinaryFnSeries<T,
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
     UnaryFnSeries<T, std::decay_t<decltype(operand_series)>>{operand_series};

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

} // namespace details

class MultiplyMethod
: public details::BinaryArithmeticMethod<MultiplyMethod, std::multiplies<>> {
public:
  using details::BinaryArithmeticMethod<MultiplyMethod, std::multiplies<>>::
   BinaryArithmeticMethod;

  auto multiplicand() const noexcept -> const ScreenerMethod&;

  auto multiplier() const noexcept -> const ScreenerMethod&;
};

class DivideMethod
: public details::BinaryArithmeticMethod<DivideMethod, std::divides<>> {
public:
  using details::BinaryArithmeticMethod<DivideMethod,
                                        std::divides<>>::BinaryArithmeticMethod;

  auto dividend() const noexcept -> const ScreenerMethod&;

  auto divisor() const noexcept -> const ScreenerMethod&;
};

class AddMethod
: public details::BinaryArithmeticMethod<AddMethod, std::plus<>> {
public:
  using details::BinaryArithmeticMethod<AddMethod,
                                        std::plus<>>::BinaryArithmeticMethod;

  auto augend() const noexcept -> const ScreenerMethod&;

  auto addend() const noexcept -> const ScreenerMethod&;
};

class SubtractMethod
: public details::BinaryArithmeticMethod<SubtractMethod, std::minus<>> {
public:
  using details::BinaryArithmeticMethod<SubtractMethod,
                                        std::minus<>>::BinaryArithmeticMethod;

  auto minuend() const noexcept -> const ScreenerMethod&;

  auto subtrahend() const noexcept -> const ScreenerMethod&;
};

class NegateMethod : public details::UnaryArithmeticMethod<std::negate<>> {
public:
  using details::UnaryArithmeticMethod<std::negate<>>::UnaryArithmeticMethod;
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

class PercentageMethod
: public details::BinaryArithmeticMethod<PercentageMethod,
                                         details::Percentages<>> {
public:
  using details::BinaryArithmeticMethod<
   PercentageMethod,
   details::Percentages<>>::BinaryArithmeticMethod;

  auto total() const noexcept -> const ScreenerMethod&;

  auto percent() const noexcept -> const ScreenerMethod&;
};

} // namespace pludux::screener

#endif
