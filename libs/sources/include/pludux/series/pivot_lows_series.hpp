#ifndef PLUDUX_PLUDUX_SERIES_PIVOT_LOWS_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_PIVOT_LOWS_SERIES_HPP

#include <cstddef>
#include <limits>
#include <utility>

namespace pludux {

template<typename TSeries>
class PivotLowsSeries {
public:
  using ValueType = typename TSeries::ValueType;

  PivotLowsSeries(TSeries series, std::size_t range)
  : series_{std::move(series)}
  , range_{range}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    if(index < range_ || index >= series_.size() - range_) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    auto low_index = index;
    auto value = series_[index];
    auto low = value;

    for(std::size_t i = 1; i <= range_; ++i) {
      if(series_[index - i] <= low) {
        low = series_[index - i];
        low_index = index - i;
      }
    }

    for(std::size_t i = 1; i <= range_; ++i) {
      if(series_[index + i] <= low) {
        low = series_[index + i];
        low_index = index + i;
      }
    }

    if(low_index == index) {
      return low;
    }

    return std::numeric_limits<ValueType>::quiet_NaN();
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size();
  }

private:
  TSeries series_;
  std::size_t range_;
};

} // namespace pludux

#endif