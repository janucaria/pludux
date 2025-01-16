#ifndef PLUDUX_PLUDUX_SERIES_STOCH_RSI_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_STOCH_RSI_SERIES_HPP

#include <cstddef>
#include <limits>
#include <utility>

#include "rsi_series.hpp"
#include "sma_series.hpp"

namespace pludux {

template<typename TSeries>
class StochRsiSeries {
public:
  using ValueType = RsiSeries<TSeries>::ValueType;

  StochRsiSeries(RsiSeries<TSeries> rsi_series, std::size_t k_period)
  : rsi_series_{rsi_series}
  , k_period_{k_period}
  {
  }

  explicit StochRsiSeries(TSeries series,
                          std::size_t rsi_period,
                          std::size_t k_period)
  : StochRsiSeries{RsiSeries{std::move(series), rsi_period}, k_period}
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
      const auto rsi_i = rsi_series_[i];

      highest_high = std::max(highest_high, rsi_i);
      lowest_low = std::min(lowest_low, rsi_i);
    }

    const auto rsi = rsi_series_[index];
    const auto stoch = 100 * (rsi - lowest_low) / (highest_high - lowest_low);

    return stoch;
  }

  auto size() const noexcept -> std::size_t
  {
    return rsi_series_.size();
  }

  auto k(std::size_t k_smooth) const noexcept -> SmaSeries<StochRsiSeries>
  {
    return SmaSeries{*this, k_smooth};
  }

  auto d(std::size_t k_smooth,
         std::size_t d_period) const noexcept -> SmaSeries<StochRsiSeries>
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
  RsiSeries<TSeries> rsi_series_;
  std::size_t k_period_;
};

} // namespace pludux

#endif