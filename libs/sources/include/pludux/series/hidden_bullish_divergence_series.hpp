#ifndef PLUDUX_PLUDUX_SERIES_HIDDEN_BULLISH_DIVERGENCE_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_HIDDEN_BULLISH_DIVERGENCE_SERIES_HPP

#include <algorithm>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include <pludux/series/pivot_lows_series.hpp>

import pludux.series.series_output;

namespace pludux {

template<typename TSeries, typename USeries>
class HiddenBullishDivergenceSeries {
public:
  using ValueType = std::ptrdiff_t;

  HiddenBullishDivergenceSeries(TSeries signal_series,
                                USeries reference_series,
                                std::size_t pivot_range = 5,
                                std::size_t lookback_range = 60)
  : pivot_range_{pivot_range}
  , lookback_range_{lookback_range}
  , signal_series_{std::move(signal_series)}
  , reference_series_{std::move(reference_series)}
  {
  }

  auto operator[](std::size_t index) const noexcept -> SeriesOutput<ValueType>
  {
    const auto& signal = signal_series_;
    const auto& reference = reference_series_;

    const auto signal_low_index = static_cast<ValueType>(index);
    const auto signal_low = get_lows(signal, signal_low_index);

    if(std::isnan(signal_low)) {
      return -1;
    }

    auto signal_prev_low_index = signal_low_index;
    const auto lookback_range = std::min(lookback_range_, signal.size());
    for(auto i = 1; i < lookback_range; ++i) {
      const auto signal_index = index + i;
      const auto signal_pivot_low = get_lows(signal, signal_index);
      if(!std::isnan(signal_pivot_low)) {
        signal_prev_low_index = signal_index;
        break;
      }
    }

    if(signal_prev_low_index == signal_low_index) {
      return -1;
    }

    const auto signal_prev_low = get_lows(signal, signal_prev_low_index);
    const auto reference_at_signal_prev_low = reference[signal_prev_low_index];
    const auto reference_at_signal_low = reference[signal_low_index];
    const auto is_hidden_bullish_divergence =
     signal_low < signal_prev_low &&
     reference_at_signal_low > reference_at_signal_prev_low;

    return is_hidden_bullish_divergence ? signal_prev_low_index : -1;
  }

  auto size() const noexcept -> std::size_t
  {
    assert(signal_series_.size() == reference_series_.size());
    return signal_series_.size();
  }

  auto pivot_range() const noexcept -> std::size_t
  {
    return pivot_range_;
  }

  auto lookback_range() const noexcept -> std::size_t
  {
    return lookback_range_;
  }

  auto signal_series() const noexcept -> const TSeries&
  {
    return signal_series_;
  }

  auto reference_series() const noexcept -> const USeries&
  {
    return reference_series_;
  }

  template<typename Series>
  auto get_lows(const Series& series, std::size_t index) const noexcept ->
   typename Series::ValueType
  {
    if(index < pivot_range_) {
      auto value = series[index];

      for(std::size_t i = 1; i <= index; ++i) {
        const auto neighbor_index = index - i;
        const auto& neighbor_value = series[neighbor_index];
        if(neighbor_value <= value) {
          return std::numeric_limits<typename Series::ValueType>::quiet_NaN();
        }
      }

      for(std::size_t i = 1; i <= pivot_range_; ++i) {
        const auto neighbor_index = index + i;
        const auto& neighbor_value = series[neighbor_index];
        if(neighbor_value <= value) {
          return std::numeric_limits<typename Series::ValueType>::quiet_NaN();
        }
      }

      return value;
    }

    const auto series_size = series.size();
    if(index >= series_size - pivot_range_) {
      auto value = series[index];

      for(std::size_t i = 1; i <= pivot_range_; ++i) {
        const auto neighbor_index = index - i;
        const auto& neighbor_value = series[neighbor_index];
        if(neighbor_value <= value) {
          return std::numeric_limits<typename Series::ValueType>::quiet_NaN();
        }
      }

      for(int i = 1, ii = series_size - index; i < ii; ++i) {
        const auto neighbor_index = index + i;
        const auto& neighbor_value = series[neighbor_index];
        if(neighbor_value <= value) {
          return std::numeric_limits<typename Series::ValueType>::quiet_NaN();
        }
      }

      return value;
    }

    return PivotLowsSeries{series, pivot_range_}[index];
  }

private:
  std::size_t pivot_range_;
  std::size_t lookback_range_;
  TSeries signal_series_;
  USeries reference_series_;
};

} // namespace pludux

#endif