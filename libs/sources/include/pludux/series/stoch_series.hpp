#ifndef PLUDUX_PLUDUX_SERIES_STOCH_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_STOCH_SERIES_HPP

#include <cstddef>
#include <limits>
#include <utility>

#include "sma_series.hpp"

namespace pludux {

template<typename THighSeries, typename TLowSeries, typename TCloseSeries>
class StochSeries {
public:
  using ValueType = std::common_type_t<typename THighSeries::ValueType,
                                       typename TLowSeries::ValueType,
                                       typename TCloseSeries::ValueType>;

  StochSeries(THighSeries highs,
                       TLowSeries lows,
                       TCloseSeries closes,
                       std::size_t period)
  : highs_{std::move(highs)}
  , lows_{std::move(lows)}
  , closes_{std::move(closes)}
  , k_period_{period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    const auto series_size = size();
    const auto nan_index = series_size - k_period_;
    if(index > nan_index) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    const auto begin = index;
    const auto end = index + k_period_;

    auto highest_high = std::numeric_limits<ValueType>::min();
    auto lowest_low = std::numeric_limits<ValueType>::max();

    for(auto i = begin; i < end; ++i) {
      const auto high = highs_[i];
      const auto low = lows_[i];

      highest_high = std::max(highest_high, high);
      lowest_low = std::min(lowest_low, low);
    }

    const auto close = closes_[index];
    const auto stoch = 100 * (close - lowest_low) / (highest_high - lowest_low);

    return stoch;
  }

  auto size() const noexcept -> std::size_t
  {
    assert(highs_.size() == lows_.size());
    assert(highs_.size() == closes_.size());

    return highs_.size();
  }

  auto k(std::size_t k_smooth) const noexcept -> SmaSeries<StochSeries>
  {
    return SmaSeries{*this, k_smooth};
  }

  auto d(std::size_t k_smooth,
         std::size_t d_period) const noexcept -> SmaSeries<StochSeries>
  {
    return d(k(k_smooth), d_period);
  }

  template<typename TKSeries>
  auto d(TKSeries k_series,
         std::size_t d_period) const noexcept -> SmaSeries<TKSeries>
  {
    return SmaSeries{k_series, d_period};
  }

private:
  THighSeries highs_;
  TLowSeries lows_;
  TCloseSeries closes_;
  std::size_t k_period_;
};

} // namespace pludux

#endif