module;

#include <cstddef>
#include <utility>
#include <limits>

export module pludux.series.lookback_series;

import pludux.series.series_output;

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

  auto operator[](std::size_t lookback) const -> SeriesOutput<ValueType>
  {
    if(lookback >= series_.size()) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    return series_[lookback + period_];
  }

  auto period() const noexcept -> std::size_t
  {
    return period_;
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size() - period_;
  }

  auto series() const noexcept -> const TSeries&
  {
    return series_;
  }

private:
  TSeries series_;
  std::size_t period_;
};

} // namespace pludux
