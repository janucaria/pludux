#ifndef PLUDUX_PLUDUX_SERIES_STOCH_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_STOCH_SERIES_HPP

#include <cstddef>
#include <limits>
#include <utility>

#include <pludux/series/series_output.hpp>

#include "binary_fn_series.hpp"
#include "repeat_series.hpp"
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
              std::size_t k_period,
              std::size_t k_smooth,
              std::size_t d_period)
  : high_{std::move(highs)}
  , low_{std::move(lows)}
  , close_{std::move(closes)}
  , k_period_{k_period}
  , k_smooth_{k_smooth}
  , d_period_{d_period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> SeriesOutput<ValueType>
  {
    const auto series_size = size();
    const auto nan_index = series_size - k_period_ + 1;
    const auto last_k_index = index + k_smooth_ - 1;
    const auto last_d_index = last_k_index + d_period_ - 1;

    auto result_k = std::numeric_limits<ValueType>::quiet_NaN();
    if(last_k_index < nan_index) {
      auto sum_k_stoch = ValueType{0};
      for(std::size_t k_smooth_i = 0; k_smooth_i < k_smooth_; ++k_smooth_i) {
        const auto k_index = index + k_smooth_i;
        const auto begin = k_index;
        const auto end = k_index + k_period_;

        auto highest_high = std::numeric_limits<ValueType>::min();
        auto lowest_low = std::numeric_limits<ValueType>::max();

        for(auto i = begin; i < end; ++i) {
          const auto high = static_cast<ValueType>(high_[i]);
          const auto low = static_cast<ValueType>(low_[i]);

          highest_high = std::max(highest_high, high);
          lowest_low = std::min(lowest_low, low);
        }

        const auto stoch =
         100 * (close_[k_index] - lowest_low) / (highest_high - lowest_low);
        sum_k_stoch += stoch;
      }

      result_k = sum_k_stoch / k_smooth_;
    }

    auto result_d = std::numeric_limits<ValueType>::quiet_NaN();
    if(last_d_index < nan_index) {
      auto sum_k_smooth = result_k;
      for(std::size_t d_smooth_i = 1; d_smooth_i < d_period_; ++d_smooth_i) {
        const auto k_smooth_size = k_smooth_ + d_smooth_i;

        auto sum_stoch = ValueType{0};
        for(std::size_t k_smooth_i = d_smooth_i; k_smooth_i < k_smooth_size;
            ++k_smooth_i) {
          const auto k_index = index + k_smooth_i;

          const auto begin = k_index;
          const auto end = k_index + k_period_;

          auto highest_high = std::numeric_limits<ValueType>::min();
          auto lowest_low = std::numeric_limits<ValueType>::max();

          for(auto i = begin; i < end; ++i) {
            const auto high = static_cast<ValueType>(high_[i]);
            const auto low = static_cast<ValueType>(low_[i]);

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

      result_d = sum_k_smooth / d_period_;
    }

    return {{result_k, result_d},
            {{OutputName::StochasticK, 0}, {OutputName::StochasticD, 1}}};
  }

  auto size() const noexcept -> std::size_t
  {
    assert(high_.size() == low_.size());
    assert(high_.size() == close_.size());

    return close_.size();
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
  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
};

} // namespace pludux

#endif