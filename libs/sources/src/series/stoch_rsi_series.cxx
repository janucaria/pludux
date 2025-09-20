module;

#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <utility>

export module pludux:series.stoch_rsi_series;

import :series.series_output;
import :series.rsi_series;

export namespace pludux {

template<typename TSeries>
class StochRsiSeries {
public:
  using ValueType = RsiSeries<TSeries>::ValueType;

  StochRsiSeries(RsiSeries<TSeries> rsi_series,
                 std::size_t k_period,
                 std::size_t k_smooth,
                 std::size_t d_period)
  : rsi_{rsi_series}
  , k_period_{k_period}
  , k_smooth_{k_smooth}
  , d_period_{d_period}
  {
  }

  explicit StochRsiSeries(TSeries rsi_input,
                          std::size_t rsi_period,
                          std::size_t k_period,
                          std::size_t k_smooth,
                          std::size_t d_period)
  : StochRsiSeries{
     RsiSeries{std::move(rsi_input), rsi_period}, k_period, k_smooth, d_period}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto series_size = self.size();
    const auto nan_lookback = series_size - self.k_period_ + 1;
    const auto last_k_lookback = lookback + self.k_smooth_ - 1;
    const auto last_d_lookback = last_k_lookback + self.d_period_ - 1;

    auto result_k = std::numeric_limits<ValueType>::quiet_NaN();
    if(last_k_lookback < nan_lookback) {
      auto sum_stoch = ValueType{0};
      for(std::size_t k_smooth_i = 0; k_smooth_i < self.k_smooth_;
          ++k_smooth_i) {
        const auto k_lookback = lookback + k_smooth_i;
        const auto begin = k_lookback;
        const auto end = k_lookback + self.k_period_;

        auto highest_high = std::numeric_limits<ValueType>::min();
        auto lowest_low = std::numeric_limits<ValueType>::max();

        for(auto i = begin; i < end; ++i) {
          const auto rsi_i = static_cast<ValueType>(self.rsi_[i]);

          highest_high = std::max(highest_high, rsi_i);
          lowest_low = std::min(lowest_low, rsi_i);
        }

        const auto rsi = self.rsi_[k_lookback];
        const auto stoch =
         100 * (rsi - lowest_low) / (highest_high - lowest_low);

        sum_stoch += stoch;
      }

      result_k = sum_stoch / self.k_smooth_;
    }

    auto result_d = std::numeric_limits<ValueType>::quiet_NaN();
    if(last_d_lookback < nan_lookback) {
      auto sum_k_smooth = result_k;
      for(std::size_t d_smooth_i = 1; d_smooth_i < self.d_period_;
          ++d_smooth_i) {
        const auto k_smooth_size = self.k_smooth_ + d_smooth_i;

        auto sum_stoch = ValueType{0};
        for(std::size_t k_smooth_i = d_smooth_i; k_smooth_i < k_smooth_size;
            ++k_smooth_i) {
          const auto k_lookback = lookback + k_smooth_i;
          const auto begin = k_lookback;
          const auto end = k_lookback + self.k_period_;

          auto highest_high = std::numeric_limits<ValueType>::min();
          auto lowest_low = std::numeric_limits<ValueType>::max();

          for(auto i = begin; i < end; ++i) {
            const auto rsi_i = static_cast<ValueType>(self.rsi_[i]);

            highest_high = std::max(highest_high, rsi_i);
            lowest_low = std::min(lowest_low, rsi_i);
          }

          const auto rsi = self.rsi_[k_lookback];
          const auto stoch =
           100 * (rsi - lowest_low) / (highest_high - lowest_low);
          sum_stoch += stoch;
        }

        const auto k = sum_stoch / self.k_smooth_;
        sum_k_smooth += k;
      }

      result_d = sum_k_smooth / self.d_period_;
    }

    return {{result_k, result_d},
            {{OutputName::StochasticK, 0}, {OutputName::StochasticD, 1}}};
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.rsi_.size();
  }

private:
  RsiSeries<TSeries> rsi_;
  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
};

} // namespace pludux
