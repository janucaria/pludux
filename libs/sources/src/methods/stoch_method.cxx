module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.stoch_method;

import :methods.value_method;

export namespace pludux {

template<typename TParameterMethod = ValueMethod>
class StochMethod {
public:
  StochMethod(std::size_t k_period, std::size_t k_smooth, std::size_t d_period)
  : StochMethod{ValueMethod{static_cast<double>(k_period)},
                ValueMethod{static_cast<double>(k_smooth)},
                ValueMethod{static_cast<double>(d_period)}}
  {
  }

  StochMethod(TParameterMethod k_period,
              TParameterMethod k_smooth,
              TParameterMethod d_period)
    requires(!std::is_arithmetic_v<TParameterMethod>)
  : k_period_{std::move(k_period)}
  , k_smooth_{std::move(k_smooth)}
  , d_period_{std::move(d_period)}
  {
  }

  auto operator==(const StochMethod& other) const noexcept -> bool = default;

  auto k_period(this const StochMethod& self) noexcept
   -> const TParameterMethod&
  {
    return self.k_period_;
  }

  void k_period(this StochMethod& self, TParameterMethod k_period) noexcept
  {
    self.k_period_ = k_period;
  }

  auto k_smooth(this const StochMethod& self) noexcept
   -> const TParameterMethod&
  {
    return self.k_smooth_;
  }

  void k_smooth(this StochMethod& self, TParameterMethod k_smooth) noexcept
  {
    self.k_smooth_ = k_smooth;
  }

  auto d_period(this const StochMethod& self) noexcept
   -> const TParameterMethod&
  {
    return self.d_period_;
  }

  void d_period(this StochMethod& self, TParameterMethod d_period) noexcept
  {
    self.d_period_ = d_period;
  }

private:
  TParameterMethod k_period_;
  TParameterMethod k_smooth_;
  TParameterMethod d_period_;
};

} // namespace pludux
