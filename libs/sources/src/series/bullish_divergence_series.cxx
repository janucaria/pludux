module;

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

export module pludux:series.bullish_divergence_series;

import :series.series_output;
import :series.pivot_lows_series;

export namespace pludux {

template<typename TSeries, typename USeries>
class BullishDivergenceSeries {
public:
  using ValueType = std::ptrdiff_t;

  BullishDivergenceSeries(TSeries signal_series,
                          USeries reference_series,
                          std::size_t pivot_range = 5,
                          std::size_t lookback_range = 60)
  : pivot_range_{pivot_range}
  , lookback_range_{lookback_range}
  , signal_series_{std::move(signal_series)}
  , reference_series_{std::move(reference_series)}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto& signal = self.signal_series_;
    const auto& reference = self.reference_series_;

    const auto signal_low_lookback = static_cast<ValueType>(lookback);
    const auto signal_low = self.get_lows(signal, signal_low_lookback);

    if(std::isnan(signal_low)) {
      return -1;
    }

    auto signal_prev_low_lookback = signal_low_lookback;
    const auto lookback_range = std::min(self.lookback_range_, signal.size());
    for(auto i = 1; i < lookback_range; ++i) {
      const auto signal_index = lookback + i;
      const auto signal_pivot_low = self.get_lows(signal, signal_index);
      if(!std::isnan(signal_pivot_low)) {
        signal_prev_low_lookback = signal_index;
        break;
      }
    }

    if(signal_prev_low_lookback == signal_low_lookback) {
      return -1;
    }

    const auto signal_prev_low =
     self.get_lows(signal, signal_prev_low_lookback);
    const auto reference_at_signal_prev_low =
     reference[signal_prev_low_lookback];
    const auto reference_at_signal_low = reference[signal_low_lookback];
    const auto is_bullish_divergence =
     signal_low > signal_prev_low &&
     reference_at_signal_low < reference_at_signal_prev_low;

    return is_bullish_divergence ? signal_prev_low_lookback : -1;
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    assert(self.signal_series_.size() == self.reference_series_.size());
    return self.signal_series_.size();
  }

  auto pivot_range(this const auto& self) noexcept -> std::size_t
  {
    return self.pivot_range_;
  }

  auto lookback_range(this const auto& self) noexcept -> std::size_t
  {
    return self.lookback_range_;
  }

  auto signal_series(this const auto& self) noexcept -> const TSeries&
  {
    return self.signal_series_;
  }

  auto reference_series(this const auto& self) noexcept -> const USeries&
  {
    return self.reference_series_;
  }

private:
  std::size_t pivot_range_;
  std::size_t lookback_range_;
  TSeries signal_series_;
  USeries reference_series_;

  template<typename Series>
  auto get_lows(this const auto& self,
                const Series& series,
                std::size_t lookback) noexcept -> typename Series::ValueType
  {
    if(lookback < self.pivot_range_) {
      auto value = series[lookback];

      for(std::size_t i = 1; i <= lookback; ++i) {
        const auto neighbor_index = lookback - i;
        const auto& neighbor_value = series[neighbor_index];
        if(neighbor_value <= value) {
          return std::numeric_limits<typename Series::ValueType>::quiet_NaN();
        }
      }

      for(std::size_t i = 1; i <= self.pivot_range_; ++i) {
        const auto neighbor_index = lookback + i;
        const auto& neighbor_value = series[neighbor_index];
        if(neighbor_value <= value) {
          return std::numeric_limits<typename Series::ValueType>::quiet_NaN();
        }
      }

      return value;
    }

    const auto series_size = series.size();
    if(lookback >= series_size - self.pivot_range_) {
      auto value = series[lookback];

      for(std::size_t i = 1; i <= self.pivot_range_; ++i) {
        const auto neighbor_index = lookback - i;
        const auto& neighbor_value = series[neighbor_index];
        if(neighbor_value <= value) {
          return std::numeric_limits<typename Series::ValueType>::quiet_NaN();
        }
      }

      for(int i = 1, ii = series_size - lookback; i < ii; ++i) {
        const auto neighbor_index = lookback + i;
        const auto& neighbor_value = series[neighbor_index];
        if(neighbor_value <= value) {
          return std::numeric_limits<typename Series::ValueType>::quiet_NaN();
        }
      }

      return value;
    }

    return PivotLowsSeries{series, self.pivot_range_}[lookback];
  }
};

} // namespace pludux
