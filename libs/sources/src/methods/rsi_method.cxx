module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.rsi_method;

import :methods.ohlcv_method;
import :methods.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TPeriodMethod = ValueMethod>
class RsiMethod {
public:
  RsiMethod()
  : RsiMethod{14}
  {
  }

  explicit RsiMethod(std::size_t period)
  : RsiMethod{TSourceMethod{}, period}
  {
  }

  explicit RsiMethod(TSourceMethod source, std::size_t period)
  : RsiMethod{std::move(source), ValueMethod{static_cast<double>(period)}}
  {
  }

  RsiMethod(TSourceMethod source, TPeriodMethod period)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const RsiMethod& other) const noexcept -> bool = default;

  auto source(this const RsiMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this RsiMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RsiMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this RsiMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TSourceMethod source_;
  TPeriodMethod period_;
};

RsiMethod() -> RsiMethod<CloseMethod>;
RsiMethod(std::size_t) -> RsiMethod<CloseMethod>;

} // namespace pludux
