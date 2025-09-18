module;

#include <cstddef>
#include <utility>

export module pludux:screener.hidden_bullish_divergence_method;

import :asset_snapshot;
import :screener.screener_method;
import :screener.ohlcv_method;
import :screener.ta_with_period_method;

export namespace pludux::screener {

class HiddenBullishDivergenceMethod {
public:
  HiddenBullishDivergenceMethod()
  : HiddenBullishDivergenceMethod{RsiMethod{}, CloseMethod{}, 5, 60}
  {
  }

  HiddenBullishDivergenceMethod(ScreenerMethod signal,
                                ScreenerMethod reference,
                                std::size_t pivot_range,
                                std::size_t lookback_range)
  : pivot_range_{pivot_range}
  , lookback_range_{lookback_range}
  , signal_{signal}
  , reference_{reference}
  {
  }

  auto operator==(const HiddenBullishDivergenceMethod& other) const noexcept
   -> bool = default;

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    const auto signal_series = signal_(asset_data);
    const auto reference_series = reference_(asset_data);

    const auto hidden_bullish_divergence_series = HiddenBullishDivergenceSeries{
     signal_series,
     reference_series,
     pivot_range_,
     lookback_range_,
    };

    return hidden_bullish_divergence_series;
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
  std::size_t pivot_range_;
  std::size_t lookback_range_;

  ScreenerMethod signal_;
  ScreenerMethod reference_;
};

} // namespace pludux::screener