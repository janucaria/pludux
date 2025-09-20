module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.lookback_series;

import :series.series_output;

export namespace pludux {

template<typename TSeries>
class LookbackSeries {
public:
  using ValueType = typename TSeries::ValueType;

  LookbackSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  LookbackSeries(LookbackSeries<TSeries> series, std::size_t period)
  : LookbackSeries{series.series_, series.period_ + period}
  {
  }

  template<typename USeries>
  LookbackSeries(LookbackSeries<USeries> other_series, std::size_t period)
  : LookbackSeries{other_series.series(), other_series.period() + period}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    if(lookback >= self.series_.size()) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    return self.series_[lookback + self.period_];
  }

  auto period(this const auto& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.series_.size() - self.period_;
  }

  auto series(this const auto& self) noexcept -> const TSeries&
  {
    return self.series_;
  }

private:
  TSeries series_;
  std::size_t period_;
};

} // namespace pludux
