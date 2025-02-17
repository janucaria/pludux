#ifndef PLUDUX_PLUDUX_SCREENER_Arithmetic_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_Arithmetic_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>

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

private:
  ScreenerMethod operand_;
  std::size_t offset_;
};

template<typename T = void>
struct Percentages {
  auto operator()(T a, T b) const -> T
  {
    return a / b * 100.0;
  }
};

template<>
struct Percentages<void> {
  auto operator()(auto a, auto b) const
  {
    return a * (b / 100.0);
  }
};

} // namespace details

class MultiplyMethod
: public details::BinaryArithmeticMethod<MultiplyMethod, std::multiplies<>> {
  using details::BinaryArithmeticMethod<MultiplyMethod, std::multiplies<>>::
   BinaryArithmeticMethod;
};

class DivideMethod
: public details::BinaryArithmeticMethod<DivideMethod, std::divides<>> {
  using details::BinaryArithmeticMethod<DivideMethod,
                                        std::divides<>>::BinaryArithmeticMethod;
};

class AddMethod
: public details::BinaryArithmeticMethod<AddMethod, std::plus<>> {
  using details::BinaryArithmeticMethod<AddMethod,
                                        std::plus<>>::BinaryArithmeticMethod;
};

class SubtractMethod
: public details::BinaryArithmeticMethod<SubtractMethod, std::minus<>> {
  using details::BinaryArithmeticMethod<SubtractMethod,
                                        std::minus<>>::BinaryArithmeticMethod;
};

class NegateMethod : public details::UnaryArithmeticMethod<std::negate<>> {
  using details::UnaryArithmeticMethod<std::negate<>>::UnaryArithmeticMethod;
};

class PercentageMethod
: public details::BinaryArithmeticMethod<PercentageMethod,
                                         details::Percentages<>> {
  using details::BinaryArithmeticMethod<
   PercentageMethod,
   details::Percentages<>>::BinaryArithmeticMethod;
};

} // namespace pludux::screener

#endif
