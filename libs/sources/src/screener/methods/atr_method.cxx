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
            double multiplier = 1.0)
  : period_{period}
  , multiplier_{multiplier}
  , high_{std::move(high)}
  , low_{std::move(low)}
  , close_{std::move(close)}
  {
  }

  auto operator==(const AtrMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    const auto high_series = self.high_(asset_data);
    const auto low_series = self.low_(asset_data);
    const auto close_series = self.close_(asset_data);

    const auto atr = AtrSeries{
     high_series, low_series, close_series, self.period_, self.multiplier_};

    return atr;
  }

  auto period(this const auto& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this auto& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

  auto multiplier(this const auto& self) noexcept -> double
  {
    return self.multiplier_;
  }

  void multiplier(this auto& self, double multiplier) noexcept
  {
    self.multiplier_ = multiplier;
  }

  auto high(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.high_;
  }

  void high(this auto& self, ScreenerMethod high) noexcept
  {
    self.high_ = std::move(high);
  }

  auto low(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.low_;
  }

  void low(this auto& self, ScreenerMethod low) noexcept
  {
    self.low_ = std::move(low);
  }

  auto close(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.close_;
  }

  void close(this auto& self, ScreenerMethod close) noexcept
  {
    self.close_ = std::move(close);
  }

private:
  std::size_t period_;
  double multiplier_;

  ScreenerMethod high_;
  ScreenerMethod low_;
  ScreenerMethod close_;
};

} // namespace pludux::screener