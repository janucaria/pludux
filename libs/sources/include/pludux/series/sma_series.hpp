#ifndef PLUDUX_PLUDUX_SERIES_SMA_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_SMA_SERIES_HPP

#include <cstddef>
#include <limits>
#include <utility>

namespace pludux {

template<typename TSeries>
class SmaSeries {
public:
  using ValueType = typename TSeries::ValueType;

  SmaSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
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

private:
  TSeries series_;
  std::size_t period_;
};

} // namespace pludux

#endif