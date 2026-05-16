module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

export module pludux:series.ema_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
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
  : source_{std::move(source)}
  , period_{period}
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

  auto period(this const EmaMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this EmaMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux
