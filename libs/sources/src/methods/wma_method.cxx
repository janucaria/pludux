module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.wma_method;

import :methods.ohlcv_method;
import :methods.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TPeriodMethod = ValueMethod>
class WmaMethod {
public:
  WmaMethod()
  : WmaMethod{20}
  {
  }

  explicit WmaMethod(std::size_t period)
  : WmaMethod{TSourceMethod{}, period}
  {
  }

  explicit WmaMethod(TSourceMethod source, std::size_t period)
  : WmaMethod{std::move(source), ValueMethod{static_cast<double>(period)}}
  {
  }

  WmaMethod(TSourceMethod source, TPeriodMethod period)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const WmaMethod& other) const noexcept -> bool = default;

  auto source(this const WmaMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this WmaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const WmaMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this WmaMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TSourceMethod source_;
  TPeriodMethod period_;
};

} // namespace pludux
