module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.lowest_method;

import :methods.ohlcv_method;
import :methods.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TPeriodMethod = ValueMethod>
class LowestMethod {
public:
  LowestMethod()
  : LowestMethod{14}
  {
  }

  explicit LowestMethod(std::size_t period)
  : LowestMethod{TSourceMethod{}, period}
  {
  }

  explicit LowestMethod(TSourceMethod source, std::size_t period)
  : LowestMethod{std::move(source), ValueMethod{static_cast<double>(period)}}
  {
  }

  LowestMethod(TSourceMethod source, TPeriodMethod period)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const LowestMethod& other) const noexcept -> bool = default;

  auto source(this const LowestMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this LowestMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const LowestMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this LowestMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TSourceMethod source_;
  TPeriodMethod period_;
};

} // namespace pludux
