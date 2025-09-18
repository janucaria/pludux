#ifndef PLUDUX_PLUDUX_SERIES_RVOL_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_RVOL_SERIES_HPP

#include <cstddef>
#include <limits>
#include <utility>

import pludux.series.series_output;
import pludux.series.sma_series;

namespace pludux {

template<typename TSeries>
class RvolSeries {
public:
  using ValueType = typename TSeries::ValueType;

  RvolSeries(TSeries volumes, std::size_t period)
  : volumes_{std::move(volumes)}
  , period_{period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> SeriesOutput<ValueType>
  {
    const auto sma_volume = SmaSeries{volumes_, period_}[index];
    const auto current_volume = volumes_[index];
    const auto rvol = current_volume / sma_volume;
    return rvol;
  }

  auto size() const noexcept -> std::size_t
  {
    return volumes_.size();
  }

private:
  TSeries volumes_;
  std::size_t period_;
};

} // namespace pludux

#endif