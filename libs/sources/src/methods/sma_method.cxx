module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.sma_method;

import :methods.ohlcv_method;
import :methods.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TPeriodMethod = ValueMethod>
class SmaMethod {
public:
  SmaMethod()
  : SmaMethod{20}
  {
  }

  explicit SmaMethod(std::size_t period)
  : SmaMethod{TSourceMethod{}, period}
  {
  }

  explicit SmaMethod(TSourceMethod source, std::size_t period)
  : SmaMethod{std::move(source), ValueMethod{static_cast<double>(period)}}
  {
  }

  SmaMethod(TSourceMethod source, TPeriodMethod period)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const SmaMethod& other) const noexcept -> bool = default;

  auto source(this const SmaMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this SmaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const SmaMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this SmaMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TSourceMethod source_;
  TPeriodMethod period_;
};

} // namespace pludux
