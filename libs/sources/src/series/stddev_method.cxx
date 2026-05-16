module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <ranges>
#include <utility>

export module pludux:series.stddev_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
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
  : source_{std::move(source)}
  , period_{period}
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

  auto period(this const StddevMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this StddevMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux
