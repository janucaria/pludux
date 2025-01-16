#ifndef PLUDUX_PLUDUX_SERIES_ATR_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_ATR_SERIES_HPP

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

#include "tr_series.hpp"
#include "rma_series.hpp"

namespace pludux {

template<typename THighSeries, typename TLowSeries, typename TCloseSeries>
class AtrSeries {
public:
  using ValueType = typename RmaSeries<
   TrSeries<THighSeries, TLowSeries, TCloseSeries>>::ValueType;

  AtrSeries(THighSeries highs,
                     TLowSeries lows,
                     TCloseSeries closes,
                     std::size_t period)
  : atr_series_{TrSeries{std::move(highs), std::move(lows), std::move(closes)}, period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    return atr_series_[index];
  }

  auto size() const noexcept -> std::size_t
  {
    return atr_series_.size();
  }

private:
  RmaSeries<TrSeries<THighSeries, TLowSeries, TCloseSeries>> atr_series_;
};

} // namespace pludux

#endif