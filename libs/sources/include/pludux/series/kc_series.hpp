#ifndef PLUDUX_PLUDUX_SERIES_KC_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_KC_SERIES_HPP

#include <cassert>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>


#include "atr_series.hpp"
#include "binary_fn_series.hpp"
#include "ema_series.hpp"

import pludux.series.series_output;

namespace pludux {

template<typename TRangeSeries, typename TMaSeries>
class KcSeries {
public:
  using ValueType = std::common_type_t<typename TRangeSeries::ValueType,
                                       typename TMaSeries::ValueType>;

  KcSeries(TMaSeries ma, TRangeSeries range, double multiplier)
  : multiplier_{multiplier}
  , ma_{std::move(ma)}
  , range_{std::move(range)}
  {
  }

  auto operator[](std::size_t index) const noexcept -> SeriesOutput<ValueType>
  {
    const auto range = range_[index] * multiplier_;
    const auto middle = ma_[index];

    const auto upper = middle + range;
    const auto lower = middle - range;

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
  double multiplier_;
  TMaSeries ma_;
  TRangeSeries range_;
};

} // namespace pludux

#endif