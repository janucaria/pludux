#ifndef PLUDUX_PLUDUX_SERIES_ATR_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_ATR_SERIES_HPP

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

#include "rma_series.hpp"
#include "tr_series.hpp"

namespace pludux {

template<typename THighSeries, typename TLowSeries, typename TCloseSeries>
class AtrSeries {
public:
  using ValueType = std::common_type_t<
   typename RmaSeries<
    TrSeries<THighSeries, TLowSeries, TCloseSeries>>::ValueType,
   double>;

  AtrSeries(THighSeries highs,
            TLowSeries lows,
            TCloseSeries closes,
            std::size_t period,
            double multiplier = 1.0)
  : atr_series_{TrSeries{std::move(highs), std::move(lows), std::move(closes)},
                period}
  , multiplier_{multiplier}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    return atr_series_[index] * multiplier_;
  }

  auto size() const noexcept -> std::size_t
  {
    return atr_series_.size();
  }

private:
  double multiplier_;
  RmaSeries<TrSeries<THighSeries, TLowSeries, TCloseSeries>> atr_series_;
};

} // namespace pludux

#endif