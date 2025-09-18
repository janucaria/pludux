#ifndef PLUDUX_PLUDUX_SERIES_RMA_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_RMA_SERIES_HPP

#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>


import pludux.series.series_output;

#include "sma_series.hpp"

namespace pludux {

template<typename TSeries>
class RmaSeries {
public:
  using ValueType = typename TSeries::ValueType;

  RmaSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  , cache_values_{}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    const auto series_size = size();

    if(cache_values_.empty()) {
      cache_values_.reserve(series_size);
    }

    const auto cache_index = series_size - 1 - index;
    if(cache_index < cache_values_.size()) {
      return cache_values_[cache_index];
    }

    const auto alpha = 1.0 / period_;
    const auto sma_index = series_size - period_;
    auto rma_value = std::numeric_limits<ValueType>::quiet_NaN();
    for(auto ii = series_size; ii > index; --ii) {
      const auto i = ii - 1;
      if(i > sma_index) {
        cache_values_.push_back(rma_value);
        continue;
      }

      if(std::isnan(rma_value)) {
        rma_value = SmaSeries{series_, period_}[i];
        cache_values_.push_back(rma_value);
        continue;
      }

      const auto series_value = series_[i];
      rma_value = series_value * alpha + rma_value * (1 - alpha);
      cache_values_.push_back(rma_value);
    }

    return rma_value;
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
  mutable std::vector<ValueType> cache_values_;
};

} // namespace pludux

#endif