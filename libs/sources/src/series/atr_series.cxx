module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:series.atr_series;

import :series.series_output;
import :series.tr_series;
import :series.rma_series;

export namespace pludux {

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

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    return self.atr_series_[lookback] * self.multiplier_;
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.atr_series_.size();
  }

private:
  double multiplier_;
  RmaSeries<TrSeries<THighSeries, TLowSeries, TCloseSeries>> atr_series_;
};

} // namespace pludux
