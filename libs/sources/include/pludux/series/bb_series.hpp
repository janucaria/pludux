#ifndef PLUDUX_PLUDUX_SERIES_BB_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_BB_SERIES_HPP

#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>


#include "binary_fn_series.hpp"
#include "sma_series.hpp"

import pludux.series.series_output;

namespace pludux {

template<typename TMaSeries>
  requires requires(TMaSeries ma) {
    { ma.input() };
    { ma.period() } -> std::same_as<std::size_t>;
  }
class BbSeries {
public:
  using ValueType = std::common_type_t<double, typename TMaSeries::ValueType>;

  BbSeries(TMaSeries ma, double stddev)
  : ma_{std::move(ma)}
  , stddev_{stddev}
  {
  }

  template<typename TSeries>
  BbSeries(TSeries input, std::size_t period, double stddev)
  : BbSeries{SmaSeries{std::move(input), period}, stddev}
  {
  }

  auto operator[](std::size_t index) const noexcept -> SeriesOutput<ValueType>
  {
    const auto period = ma_.period();
    const auto& input = ma_.input();

    const auto begin = index;
    const auto end = index + period;

    auto sum_squared_diff = 0.0;
    for(auto i = begin; i < end; ++i) {
      const auto diff = input[i] - ma_[index];
      sum_squared_diff += diff * diff;
    }

    const auto std_dev = sqrt(sum_squared_diff / period);
    const auto std_dev_scaled = std_dev * stddev_;

    const auto middle = ma_[index];
    const auto upper = middle + std_dev_scaled;
    const auto lower = middle - std_dev_scaled;

    return {{middle, upper, lower},
            {
             {OutputName::MiddleBand, 0},
             {OutputName::UpperBand, 1},
             {OutputName::LowerBand, 2},
            }};
  }

  auto size() const noexcept -> std::size_t
  {
    return ma_.size();
  }

private:
  TMaSeries ma_;
  double stddev_;
};

template<typename TMaSeries>
BbSeries(TMaSeries, double) -> BbSeries<TMaSeries>;

template<typename TSeries>
BbSeries(TSeries, std::size_t, double) -> BbSeries<SmaSeries<TSeries>>;

} // namespace pludux

#endif