module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.stddev_method;

import :methods.ohlcv_method;
import :methods.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TPeriodMethod = ValueMethod>
class StddevMethod {
public:
  StddevMethod()
  : StddevMethod{20}
  {
  }

  explicit StddevMethod(std::size_t period)
  : StddevMethod{TSourceMethod{}, period}
  {
  }

  explicit StddevMethod(TSourceMethod source, std::size_t period)
  : StddevMethod{std::move(source), ValueMethod{static_cast<double>(period)}}
  {
  }

  StddevMethod(TSourceMethod source, TPeriodMethod period)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const StddevMethod& other) const noexcept -> bool = default;

  auto source(this const StddevMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this StddevMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const StddevMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this StddevMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TSourceMethod source_;
  TPeriodMethod period_;
};

} // namespace pludux
