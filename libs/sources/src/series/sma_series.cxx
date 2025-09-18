module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.sma_series;

import :series.series_output;

export namespace pludux {

template<typename TSeries>
class SmaSeries {
public:
  using ValueType = typename TSeries::ValueType;

  SmaSeries(TSeries series, std::size_t period)
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

    auto sum = ValueType{0};
    for(auto i = begin; i < end; ++i) {
      sum += series_[i];
    }

    return sum / period_;
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
