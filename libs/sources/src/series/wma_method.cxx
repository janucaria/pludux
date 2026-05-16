module;

#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

export module pludux:series.wma_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class WmaMethod {
public:
  WmaMethod()
  : WmaMethod{20}
  {
  }

  explicit WmaMethod(std::size_t period)
  : WmaMethod{TSourceMethod{}, period}
  {
  }

  explicit WmaMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const WmaMethod& other) const noexcept -> bool = default;

  auto source(this const WmaMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this WmaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const WmaMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this WmaMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux
