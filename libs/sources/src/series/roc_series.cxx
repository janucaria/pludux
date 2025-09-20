module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.roc_series;

import :series.series_output;

export namespace pludux {

template<typename TSeries>
class RocSeries {
public:
  using ValueType = typename TSeries::ValueType;

  RocSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto series_size = self.size();
    const auto nan_index = series_size - self.period_;
    if(lookback > nan_index) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    const auto current = self.series_[lookback];
    const auto end = self.series_[lookback + self.period_];

    return 100 * (current - end) / end;
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.series_.size();
  }

private:
  TSeries series_;
  std::size_t period_;
};

} // namespace pludux
