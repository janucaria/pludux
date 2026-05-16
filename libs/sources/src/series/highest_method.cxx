module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.highest_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
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

  explicit HighestMethod(TSourceMethod method, std::size_t period)
  : source_{std::move(method)}
  , period_{period}
  {
  }

  auto operator==(const HighestMethod& other) const noexcept -> bool = default;

  auto source(this const HighestMethod& self) -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this HighestMethod& self, TSourceMethod method)
  {
    self.source_ = std::move(method);
  }

  auto period(this const HighestMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this HighestMethod& self, std::size_t period)
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux
