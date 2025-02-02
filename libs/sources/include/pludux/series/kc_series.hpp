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
#include "ref_series.hpp"

namespace pludux {

enum class KcOutput { middle, upper, lower };

template<typename TRangeSeries, typename TMaSeries>
class KcSeries {
public:
  using ValueType = std::common_type_t<typename TRangeSeries::ValueType,
                                       typename TMaSeries::ValueType>;

  KcSeries(KcOutput output, TMaSeries ma, TRangeSeries range, double multiplier)
  : output_{output}
  , multiplier_{multiplier}
  , ma_{std::move(ma)}
  , range_{std::move(range)}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    const auto range = range_[index] * multiplier_;
    const auto middle = ma_[index];

    switch(output_) {
    case KcOutput::upper:
      return middle + range;
    case KcOutput::lower:
      return middle - range;
    case KcOutput::middle:
    default:
      return middle;
    }
  }

  auto size() const noexcept -> std::size_t
  {
    return ma_.size();
  }

  auto output() const noexcept -> KcOutput
  {
    return output_;
  }

  void output(KcOutput output) noexcept
  {
    output_ = output;
  }

private:
  KcOutput output_;
  double multiplier_;
  TMaSeries ma_;
  TRangeSeries range_;
};

} // namespace pludux

#endif