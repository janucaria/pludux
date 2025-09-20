module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.pivot_lows_series;

import :series.series_output;

export namespace pludux {

template<typename TSeries>
class PivotLowsSeries {
public:
  using ValueType = typename TSeries::ValueType;

  PivotLowsSeries(TSeries series, std::size_t range)
  : series_{std::move(series)}
  , range_{range}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    if(lookback < self.range_ ||
       lookback >= self.series_.size() - self.range_) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    auto value = self.series_[lookback];

    for(std::size_t i = 1; i <= self.range_; ++i) {
      if(self.series_[lookback - i] <= value) {
        return std::numeric_limits<ValueType>::quiet_NaN();
      }
    }

    for(std::size_t i = 1; i <= self.range_; ++i) {
      if(self.series_[lookback + i] <= value) {
        return std::numeric_limits<ValueType>::quiet_NaN();
      }
    }

    return value;
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.series_.size();
  }

private:
  TSeries series_;
  std::size_t range_;
};

} // namespace pludux
