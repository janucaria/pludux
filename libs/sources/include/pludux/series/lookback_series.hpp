#ifndef PLUDUX_PLUDUX_SERIES_LOOKBACK_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_LOOKBACK_SERIES_HPP

#include <cstddef>
#include <utility>

#include <pludux/series/series_output.hpp>

namespace pludux {

template<typename TSeries>
class LookbackSeries {
public:
  using ValueType = typename TSeries::ValueType;

  LookbackSeries(TSeries series, std::ptrdiff_t periods)
  : series_{std::move(series)}
  , periods_{periods}
  {
  }

  LookbackSeries(LookbackSeries<TSeries> series, std::ptrdiff_t periods)
  : LookbackSeries{series.series_, series.periods_ + periods}
  {
  }

  template<typename USeries>
  LookbackSeries(LookbackSeries<USeries> other_series, std::ptrdiff_t periods)
  : LookbackSeries{other_series.series(), other_series.periods() + periods}
  {
  }

  auto operator[](std::size_t lookback) const -> SeriesOutput<ValueType>
  {
    if(lookback >= series_.size()) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    return series_[lookback + periods_];
  }

  auto periods() const noexcept -> std::ptrdiff_t
  {
    return periods_;
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size() - periods_;
  }

  auto series() const noexcept -> const TSeries&
  {
    return series_;
  }

private:
  TSeries series_;
  std::ptrdiff_t periods_;
};

} // namespace pludux

#endif