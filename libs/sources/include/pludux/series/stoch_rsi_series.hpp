#ifndef PLUDUX_PLUDUX_SERIES_STOCH_RSI_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_STOCH_RSI_SERIES_HPP

#include <cstddef>
#include <limits>
#include <utility>

#include "rsi_series.hpp"
#include "sma_series.hpp"
#include "stoch_series.hpp"

namespace pludux {

template<typename TSeries>
class StochRsiSeries {
public:
  using ValueType = RsiSeries<TSeries>::ValueType;

  StochRsiSeries(StochOutput output,
                 RsiSeries<TSeries> rsi_series,
                 std::size_t k_period,
                 std::size_t k_smooth,
                 std::size_t d_period)
  : rsi_{rsi_series}
  , k_period_{k_period}
  , k_smooth_{k_smooth}
  , d_period_{d_period}
  , output_{output}
  {
  }

  explicit StochRsiSeries(StochOutput output,
                          TSeries rsi_input,
                          std::size_t rsi_period,
                          std::size_t k_period,
                          std::size_t k_smooth,
                          std::size_t d_period)
  : StochRsiSeries{output,
                   RsiSeries{std::move(rsi_input), rsi_period},
                   k_period,
                   k_smooth,
                   d_period}
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
          const auto rsi_i = rsi_[i];

          highest_high = std::max(highest_high, rsi_i);
          lowest_low = std::min(lowest_low, rsi_i);
        }

        const auto rsi = rsi_[k_index];
        const auto stoch =
         100 * (rsi - lowest_low) / (highest_high - lowest_low);

        sum_stoch += stoch;
      }

      const auto k = sum_stoch / k_smooth_;
      sum_k_smooth += k;
    }

    return sum_k_smooth / d_smooth_size;
  }

  auto size() const noexcept -> std::size_t
  {
    return rsi_.size();
  }

  auto output() const noexcept -> StochOutput
  {
    return output_;
  }

  void output(StochOutput output) noexcept
  {
    output_ = output;
  }

private:
  RsiSeries<TSeries> rsi_;
  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
  StochOutput output_;
};

} // namespace pludux

#endif