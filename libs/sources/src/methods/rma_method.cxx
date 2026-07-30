module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.rma_method;

import :methods.ohlcv_method;
import :methods.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TPeriodMethod = ValueMethod>
class RmaMethod {
public:
  RmaMethod()
  : RmaMethod{20}
  {
  }

  explicit RmaMethod(std::size_t period)
  : RmaMethod{TSourceMethod{}, period}
  {
  }

  explicit RmaMethod(TSourceMethod source, std::size_t period)
  : RmaMethod{std::move(source), ValueMethod{static_cast<double>(period)}}
  {
  }

  RmaMethod(TSourceMethod source, TPeriodMethod period)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const RmaMethod& other) const noexcept -> bool = default;

  auto source(this const RmaMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this RmaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RmaMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this RmaMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TSourceMethod source_;
  TPeriodMethod period_;
};

} // namespace pludux
