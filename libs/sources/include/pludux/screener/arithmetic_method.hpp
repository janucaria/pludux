#ifndef PLUDUX_PLUDUX_SCREENER_Arithmetic_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_Arithmetic_METHOD_HPP

#include <functional>

#include <pludux/asset_data_provider.hpp>
#include <pludux/screener/screener_method.hpp>

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

  auto run_one(const AssetDataProvider& asset_data) const -> double
  {
    const auto series = run_all(asset_data);
    return series[0];
  }

  auto run_all(const AssetDataProvider& asset_data) const -> Series
  {
    const auto series1 = operand1_.run_all(asset_data);
    const auto series2 = operand2_.run_all(asset_data);

    auto result = std::vector<double>{};
    std::transform(series1.data().cbegin(),
                   series1.data().cend(),
                   series2.data().cbegin(),
                   std::back_inserter(result),
                   T{}); // T is a functor

    return Series{std::move(result)};
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

  auto run_one(const AssetDataProvider& asset_data) const -> double
  {
    const auto series = run_all(asset_data);
    return series[0];
  }

  auto run_all(const AssetDataProvider& asset_data) const -> Series
  {
    const auto series = operand_.run_all(asset_data);

    auto result = std::vector<double>{};
    std::transform(series.data().begin(),
                   series.data().end(),
                   std::back_inserter(result),
                   T{}); // T is a functor

    return Series{std::move(result)};
  }

private:
  ScreenerMethod operand_;
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

} // namespace pludux::screener

#endif
