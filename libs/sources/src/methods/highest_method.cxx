module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.highest_method;

import :methods.ohlcv_method;
import :methods.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TPeriodMethod = ValueMethod>
class HighestMethod {
public:
  HighestMethod()
  : HighestMethod{14}
  {
  }

  explicit HighestMethod(std::size_t period)
  : HighestMethod{TSourceMethod{}, period}
  {
  }

  explicit HighestMethod(TSourceMethod source, std::size_t period)
  : HighestMethod{std::move(source), ValueMethod{static_cast<double>(period)}}
  {
  }

  HighestMethod(TSourceMethod source, TPeriodMethod period)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : source_{std::move(source)}
  , period_{std::move(period)}
  {
  }

  auto operator==(const HighestMethod& other) const noexcept -> bool = default;

  auto source(this const HighestMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this HighestMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const HighestMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this HighestMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TSourceMethod source_;
  TPeriodMethod period_;
};

} // namespace pludux
