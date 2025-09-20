module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.rvol_series;

import :series.series_output;
import :series.sma_series;

export namespace pludux {

template<typename TSeries>
class RvolSeries {
public:
  using ValueType = typename TSeries::ValueType;

  RvolSeries(TSeries volumes, std::size_t period)
  : volumes_{std::move(volumes)}
  , period_{period}
  {
  }

  auto operator[](std::size_t lookback) const noexcept
   -> SeriesOutput<ValueType>
  {
    const auto sma_volume = SmaSeries{volumes_, period_}[lookback];
    const auto current_volume = volumes_[lookback];
    const auto rvol = current_volume / sma_volume;
    return rvol;
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.volumes_.size();
  }

private:
  TSeries volumes_;
  std::size_t period_;
};

} // namespace pludux
