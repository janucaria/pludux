module;

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

export module pludux:screener.atr_method;

import :asset_snapshot;
import :screener.screener_method;
import :screener.ohlcv_method;

export namespace pludux::screener {

class AtrMethod {
public:
  AtrMethod()
  : AtrMethod{HighMethod{}, LowMethod{}, CloseMethod{}, 14, 1.0}
  {
  }

  AtrMethod(ScreenerMethod high,
            ScreenerMethod low,
            ScreenerMethod close,
            std::size_t period,
            double multiplier)
  : period_{period}
  , multiplier_{multiplier}
  , high_{std::move(high)}
  , low_{std::move(low)}
  , close_{std::move(close)}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    const auto high_series = high_(asset_data);
    const auto low_series = low_(asset_data);
    const auto close_series = close_(asset_data);

    const auto atr =
     AtrSeries{high_series, low_series, close_series, period_, multiplier_};

    return atr;
  }

  auto operator==(const AtrMethod& other) const noexcept -> bool = default;

  auto period() const noexcept -> std::size_t
  {
    return period_;
  }

  void period(std::size_t period) noexcept
  {
    period_ = period;
  }

  auto multiplier() const noexcept -> double
  {
    return multiplier_;
  }

  void multiplier(double multiplier) noexcept
  {
    multiplier_ = multiplier;
  }

  auto high() const noexcept -> const ScreenerMethod&
  {
    return high_;
  }

  void high(ScreenerMethod high) noexcept
  {
    high_ = std::move(high);
  }

  auto low() const noexcept -> const ScreenerMethod&
  {
    return low_;
  }

  void low(ScreenerMethod low) noexcept
  {
    low_ = std::move(low);
  }

  auto close() const noexcept -> const ScreenerMethod&
  {
    return close_;
  }

  void close(ScreenerMethod close) noexcept
  {
    close_ = std::move(close);
  }

private:
  std::size_t period_;
  double multiplier_;

  ScreenerMethod high_;
  ScreenerMethod low_;
  ScreenerMethod close_;
};

} // namespace pludux::screener