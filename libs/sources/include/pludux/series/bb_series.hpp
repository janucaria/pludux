#ifndef PLUDUX_PLUDUX_SERIES_BB_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_BB_SERIES_HPP

#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "binary_fn_series.hpp"
#include "ref_series.hpp"
#include "sma_series.hpp"

namespace pludux {

template<typename TSeries, typename TMaSeries = SmaSeries<TSeries>>
class BbSeries {
public:
  using ValueType = std::common_type_t<typename TSeries::ValueType,
                                       typename TMaSeries::ValueType>;

  BbSeries(TSeries series, std::size_t period, double stddev)
  : series_{std::move(series)}
  , period_{period}
  , stddev_{stddev}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    const auto series_size = size();
    const auto nan_index = series_size - period_;
    if(index > nan_index) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    const auto series = RefSeries{series_};
    const auto ma_series = middle_band();

    const auto begin = index;
    const auto end = index + period_;

    auto sum_squared_diff = 0.0;
    for(auto i = begin; i < end; ++i) {
      const auto diff = series[i] - ma_series[index];
      sum_squared_diff += diff * diff;
    }

    const auto std_dev = sqrt(sum_squared_diff / period_);

    return std_dev * stddev_;
  }

  auto
  operator()() const noexcept -> std::tuple<TMaSeries,
                                            AddSeries<TMaSeries, BbSeries>,
                                            SubtractSeries<TMaSeries, BbSeries>>
  {
    return {middle_band(), upper_band(), lower_band()};
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size();
  }

  auto middle_band() const noexcept -> TMaSeries
  {
    return TMaSeries{series_, period_};
  }

  auto upper_band() const noexcept -> AddSeries<TMaSeries, BbSeries>
  {
    return AddSeries{middle_band(), *this};
  }

  auto lower_band() const noexcept -> SubtractSeries<TMaSeries, BbSeries>
  {
    return SubtractSeries{middle_band(), *this};
  }

private:
  TSeries series_;
  std::size_t period_;
  double stddev_;
};

} // namespace pludux

#endif