module;

#include <cmath>
#include <cstddef>
#include <functional>
#include <utility>

export module pludux:screener.arithmetic_method;

import :asset_snapshot;
import :screener.screener_method;

namespace pludux::screener {

template<typename, typename T>
class BinaryArithmeticMethod {
public:
  BinaryArithmeticMethod(ScreenerMethod operand1, ScreenerMethod operand2)
  : operand1_{std::move(operand1)}
  , operand2_{std::move(operand2)}
  {
  }

  auto operator==(const BinaryArithmeticMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    const auto operand1_series = self.operand1_(asset_data);
    const auto operand2_series = self.operand2_(asset_data);

    const auto result = []<typename T1, typename T2>(T1&& op1, T2&& op2) {
      return BinaryOpSeries<T, std::decay_t<T1>, std::decay_t<T2>>{
       std::forward<T1>(op1), std::forward<T2>(op2)};
    }(operand1_series, operand2_series);

    return result;
  }

  auto operand1(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand1_;
  }

  auto operand2(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand2_;
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

  auto operator==(const UnaryArithmeticMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    const auto operand_series = self.operand_(asset_data);

    auto result =
     UnaryOpSeries<T, std::decay_t<decltype(operand_series)>>{operand_series};

    return result;
  }

  auto operand(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand_;
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

  auto multiplicand(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand1();
  }

  auto multiplier(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand2();
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

  auto dividend(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand1();
  }

  auto divisor(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand2();
  }
};

export class AddMethod : public BinaryArithmeticMethod<AddMethod, std::plus<>> {
public:
  AddMethod(ScreenerMethod augend, ScreenerMethod addend)
  : BinaryArithmeticMethod<AddMethod, std::plus<>>{std::move(augend),
                                                   std::move(addend)}
  {
  }

  auto augend(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand1();
  }

  auto addend(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand2();
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

  auto minuend(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand1();
  }

  auto subtrahend(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand2();
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
  auto operator()(this const auto, T total, T percent) -> T
  {
    return total * (percent / 100.0);
  }
};

template<>
struct Percentages<void> {
  auto operator()(this const auto, auto total, auto percent)
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

  auto total(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand1();
  }

  auto percent(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand2();
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

  auto minuend(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand1();
  }

  auto subtrahend(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.operand2();
  }
};

} // namespace pludux::screener