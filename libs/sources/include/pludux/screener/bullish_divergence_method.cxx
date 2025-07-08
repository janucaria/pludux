module;

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

#include <pludux/screener/screener_method.hpp>

export module pludux.screener.bullish_divergence_method;

import pludux.asset_snapshot;
import pludux.series;
import pludux.screener.ohlcv_method;
import pludux.screener.ta_with_period_method;

namespace pludux::screener {

export class BullishDivergenceMethod {
public:
  BullishDivergenceMethod()
  : BullishDivergenceMethod{RsiMethod{}, CloseMethod{}, 5, 60, 0}
  {
  }

  BullishDivergenceMethod(ScreenerMethod signal,
                          ScreenerMethod reference,
                          std::size_t pivot_range,
                          std::size_t lookback_range,
                          std::size_t offset)
  : offset_{offset}
  , pivot_range_{pivot_range}
  , lookback_range_{lookback_range}
  , signal_{signal}
  , reference_{reference}
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    const auto signal_series = signal_(asset_data);
    const auto reference_series = reference_(asset_data);

    const auto bullish_divergence_series = BullishDivergenceSeries{
     signal_series,
     reference_series,
     pivot_range_,
     lookback_range_,
    };

    return SubSeries{PolySeries<double>{bullish_divergence_series},
                     static_cast<std::ptrdiff_t>(offset_)};
  }

  auto operator==(const BullishDivergenceMethod& other) const noexcept
   -> bool = default;

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

  void offset(std::size_t offset) noexcept
  {
    offset_ = offset;
  }

  auto pivot_range() const noexcept -> std::size_t
  {
    return pivot_range_;
  }

  void pivot_range(std::size_t pivot_range) noexcept
  {
    pivot_range_ = pivot_range;
  }

  auto lookback_range() const noexcept -> std::size_t
  {
    return lookback_range_;
  }

  void lookback_range(std::size_t lookback_range) noexcept
  {
    lookback_range_ = lookback_range;
  }

  auto signal() const noexcept -> const ScreenerMethod&
  {
    return signal_;
  }

  void signal(ScreenerMethod signal) noexcept
  {
    signal_ = std::move(signal);
  }

  auto reference() const noexcept -> const ScreenerMethod&
  {
    return reference_;
  }

  void reference(ScreenerMethod reference) noexcept
  {
    reference_ = std::move(reference);
  }

private:
  std::size_t offset_;
  std::size_t pivot_range_;
  std::size_t lookback_range_;

  ScreenerMethod signal_;
  ScreenerMethod reference_;
};

} // namespace pludux::screener
