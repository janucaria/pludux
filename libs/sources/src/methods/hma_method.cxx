module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.hma_method;

import :methods.ohlcv_method;
import :methods.value_method;
import :methods.value_method;
import :methods.wma_method;
import :methods.operators_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TPeriodMethod = ValueMethod>
class HmaMethod {
public:
  HmaMethod()
  : HmaMethod{20}
  {
  }

  explicit HmaMethod(std::size_t period)
  : HmaMethod{TSourceMethod{}, period}
  {
  }

  explicit HmaMethod(TSourceMethod source, std::size_t period)
  : HmaMethod{std::move(source), ValueMethod{static_cast<double>(period)}}
  {
  }

  HmaMethod(TSourceMethod source, TPeriodMethod period)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const HmaMethod& other) const noexcept -> bool = default;

  auto source(this const HmaMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this HmaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const HmaMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this HmaMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TSourceMethod source_;
  TPeriodMethod period_;
};

} // namespace pludux
