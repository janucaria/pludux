module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.ema_method;

import :methods.ohlcv_method;
import :methods.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TPeriodMethod = ValueMethod>
class EmaMethod {
public:
  EmaMethod()
  : EmaMethod{20}
  {
  }

  EmaMethod(std::size_t period)
  : EmaMethod{TSourceMethod{}, period}
  {
  }

  EmaMethod(TSourceMethod source, std::size_t period)
  : EmaMethod{std::move(source), ValueMethod{static_cast<double>(period)}}
  {
  }

  EmaMethod(TSourceMethod source, TPeriodMethod period)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const EmaMethod& other) const noexcept -> bool = default;

  auto source(this const EmaMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this EmaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const EmaMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this EmaMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TSourceMethod source_;
  TPeriodMethod period_;
};

} // namespace pludux
