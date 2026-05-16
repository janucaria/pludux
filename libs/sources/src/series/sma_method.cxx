module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.sma_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class SmaMethod {
public:
  SmaMethod()
  : SmaMethod{20}
  {
  }

  explicit SmaMethod(std::size_t period)
  : SmaMethod{TSourceMethod{}, period}
  {
  }

  explicit SmaMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const SmaMethod& other) const noexcept -> bool = default;

  auto source(this const SmaMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this SmaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const SmaMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this SmaMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux
