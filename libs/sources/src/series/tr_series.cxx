module;

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:series.tr_series;

import :series.series_output;

export namespace pludux {

template<typename THighSeries, typename TLowSeries, typename TCloseSeries>
class TrSeries {
public:
  using ValueType = std::common_type_t<typename THighSeries::ValueType,
                                       typename TLowSeries::ValueType,
                                       typename TCloseSeries::ValueType>;

  TrSeries(THighSeries highs, TLowSeries lows, TCloseSeries closes)
  : highs_{std::move(highs)}
  , lows_{std::move(lows)}
  , closes_{std::move(closes)}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto high = self.highs_[lookback];
    const auto low = self.lows_[lookback];

    const auto close_last_index = self.closes_.size() - 1;
    const auto prev_close = lookback == close_last_index
                             ? self.closes_[lookback]
                             : self.closes_[lookback + 1];

    const auto hl = std::abs(high - low);
    const auto hc = std::abs(high - prev_close);
    const auto lc = std::abs(low - prev_close);

    return std::max(std::max(hl, hc), lc);
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    assert(self.highs_.size() == self.lows_.size());
    assert(self.highs_.size() == self.closes_.size());

    return self.highs_.size();
  }

private:
  THighSeries highs_;
  TLowSeries lows_;
  TCloseSeries closes_;
};

} // namespace pludux
