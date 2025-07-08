module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>

export module pludux.series.stoch_series;

import pludux.series.repeat_series;
import pludux.series.binary_fn_series;
import pludux.series.sma_series;

namespace pludux {

export enum class StochOutput { k, d };

export template<typename THighSeries,
                typename TLowSeries,
                typename TCloseSeries>
class StochSeries {
public:
  using ValueType = std::common_type_t<typename THighSeries::ValueType,
                                       typename TLowSeries::ValueType,
                                       typename TCloseSeries::ValueType>;

  StochSeries(StochOutput output,
              THighSeries highs,
              TLowSeries lows,
              TCloseSeries closes,
              std::size_t k_period,
              std::size_t k_smooth,
              std::size_t d_period)
  : high_{std::move(highs)}
  , low_{std::move(lows)}
  , close_{std::move(closes)}
  , output_{output}
  , k_period_{k_period}
  , k_smooth_{k_smooth}
  , d_period_{d_period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    auto sum_k_smooth = ValueType{0};
    const auto d_smooth_size = output_ == StochOutput::d   ? k_smooth_
                               : output_ == StochOutput::k ? 1
                                                           : 1;

    for(std::size_t d_smooth_i = 0; d_smooth_i < d_smooth_size; ++d_smooth_i) {
      const auto k_smooth_size = k_smooth_ + d_smooth_i;

      auto sum_stoch = ValueType{0};
      for(std::size_t k_smooth_i = d_smooth_i; k_smooth_i < k_smooth_size;
          ++k_smooth_i) {
        const auto k_index = index + k_smooth_i;

        const auto series_size = size();
        const auto nan_index = series_size - k_period_;
        if(k_index > nan_index) {
          return std::numeric_limits<ValueType>::quiet_NaN();
        }

        const auto begin = k_index;
        const auto end = k_index + k_period_;

        auto highest_high = std::numeric_limits<ValueType>::min();
        auto lowest_low = std::numeric_limits<ValueType>::max();

        for(auto i = begin; i < end; ++i) {
          const auto high = high_[i];
          const auto low = low_[i];

          highest_high = std::max(highest_high, high);
          lowest_low = std::min(lowest_low, low);
        }

        const auto stoch =
         100 * (close_[k_index] - lowest_low) / (highest_high - lowest_low);
        sum_stoch += stoch;
      }

      const auto k = sum_stoch / k_smooth_;
      sum_k_smooth += k;
    }

    return sum_k_smooth / d_smooth_size;
  }

  auto size() const noexcept -> std::size_t
  {
    assert(high_.size() == low_.size());
    assert(high_.size() == close_.size());

    return close_.size();
  }

  auto output() const noexcept -> StochOutput
  {
    return output_;
  }

  void output(StochOutput output) noexcept
  {
    output_ = output;
  }

  auto k_period() const noexcept -> std::size_t
  {
    return k_period_;
  }

  void k_period(std::size_t k_period) noexcept
  {
    k_period_ = k_period;
  }

  auto k_smooth() const noexcept -> std::size_t
  {
    return k_smooth_;
  }

  void k_smooth(std::size_t k_smooth) noexcept
  {
    k_smooth_ = k_smooth;
  }

  auto d_period() const noexcept -> std::size_t
  {
    return d_period_;
  }

  void d_period(std::size_t d_period) noexcept
  {
    d_period_ = d_period;
  }

private:
  THighSeries high_;
  TLowSeries low_;
  TCloseSeries close_;
  StochOutput output_;
  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
};

} // namespace pludux
