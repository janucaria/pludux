#ifndef PLUDUX_PLUDUX_SERIES_WMA_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_WMA_SERIES_HPP

#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

import pludux.series.series_output;

namespace pludux {

template<typename TSeries>
class WmaSeries {
public:
  using ValueType = typename TSeries::ValueType;

  explicit WmaSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> SeriesOutput<ValueType>
  {
    const auto series_size = size();
    const auto nan_index = series_size - period_;
    if(index > nan_index) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    const auto begin = index;
    const auto end = index + period_;

    auto norm = ValueType{0};
    auto sum = ValueType{0};
    for(auto i = begin; i < end; ++i) {
      const auto weight = (period_ - (i - begin)) * period_;
      sum += series_[i] * weight;
      norm += weight;
    }

    return sum / norm;
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size();
  }

  auto input() const noexcept -> const TSeries&
  {
    return series_;
  }

  auto period() const noexcept -> std::size_t
  {
    return period_;
  }

private:
  TSeries series_;
  std::size_t period_;
};

} // namespace pludux

#endif