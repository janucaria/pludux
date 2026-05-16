module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.lowest_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
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

  explicit LowestMethod(TSourceMethod method, std::size_t period)
  : source_{std::move(method)}
  , period_{period}
  {
  }

  auto operator==(const LowestMethod& other) const noexcept -> bool = default;

  auto source(this const LowestMethod& self) -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this LowestMethod& self, TSourceMethod method)
  {
    self.source_ = std::move(method);
  }

  auto period(this const LowestMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this LowestMethod& self, std::size_t period)
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux
