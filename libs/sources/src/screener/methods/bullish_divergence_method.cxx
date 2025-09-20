module;

#include <cstddef>
#include <utility>

export module pludux:screener.bullish_divergence_method;

import :asset_snapshot;
import :screener.screener_method;
import :screener.ohlcv_method;
import :screener.ta_with_period_method;

export namespace pludux::screener {

class BullishDivergenceMethod {
public:
  BullishDivergenceMethod()
  : BullishDivergenceMethod{RsiMethod{}, CloseMethod{}, 5, 60}
  {
  }

  BullishDivergenceMethod(ScreenerMethod signal,
                          ScreenerMethod reference,
                          std::size_t pivot_range,
                          std::size_t lookback_range)
  : pivot_range_{pivot_range}
  , lookback_range_{lookback_range}
  , signal_{signal}
  , reference_{reference}
  {
  }

  auto operator==(const BullishDivergenceMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    const auto signal_series = self.signal_(asset_data);
    const auto reference_series = self.reference_(asset_data);

    const auto bullish_divergence_series = BullishDivergenceSeries{
     signal_series,
     reference_series,
     self.pivot_range_,
     self.lookback_range_,
    };

    return bullish_divergence_series;
  }

  auto pivot_range(this const auto& self) noexcept -> std::size_t
  {
    return self.pivot_range_;
  }

  void pivot_range(this auto& self, std::size_t pivot_range) noexcept
  {
    self.pivot_range_ = pivot_range;
  }

  auto lookback_range(this const auto& self) noexcept -> std::size_t
  {
    return self.lookback_range_;
  }

  void lookback_range(this auto& self, std::size_t lookback_range) noexcept
  {
    self.lookback_range_ = lookback_range;
  }

  auto signal(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.signal_;
  }

  void signal(this auto& self, ScreenerMethod signal) noexcept
  {
    self.signal_ = std::move(signal);
  }

  auto reference(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.reference_;
  }

  void reference(this auto& self, ScreenerMethod reference) noexcept
  {
    self.reference_ = std::move(reference);
  }

private:
  std::size_t pivot_range_;
  std::size_t lookback_range_;

  ScreenerMethod signal_;
  ScreenerMethod reference_;
};

} // namespace pludux::screener