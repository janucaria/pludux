module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.rvol_method;

import :methods.value_method;

export namespace pludux {

template<typename TPeriodMethod = ValueMethod>
class RvolMethod {
public:
  explicit RvolMethod(std::size_t period = 14)
  : RvolMethod{ValueMethod{static_cast<double>(period)}}
  {
  }

  explicit RvolMethod(TPeriodMethod period)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : period_{std::move(period)}
  {
  }

  auto operator==(const RvolMethod& other) const noexcept -> bool = default;

  auto period(this const RvolMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this RvolMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TPeriodMethod period_;
};

} // namespace pludux
