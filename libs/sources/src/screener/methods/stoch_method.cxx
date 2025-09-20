module;

#include <cstddef>

export module pludux:screener.stoch_method;

import :asset_snapshot;
import :screener.screener_method;

export namespace pludux::screener {

class StochMethod {
public:
  StochMethod(ScreenerMethod high,
              ScreenerMethod low,
              ScreenerMethod close,
              std::size_t k_period,
              std::size_t k_smooth,
              std::size_t d_period)
  : high_{high}
  , low_{low}
  , close_{close}
  , k_period_{k_period}
  , k_smooth_{k_smooth}
  , d_period_{d_period}
  {
  }

  auto operator==(const StochMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    const auto high_series = self.high_(asset_data);
    const auto low_series = self.low_(asset_data);
    const auto close_series = self.close_(asset_data);

    return StochSeries{high_series,
                       low_series,
                       close_series,
                       self.k_period_,
                       self.k_smooth_,
                       self.d_period_};
  }

  auto high(this const auto& self) noexcept -> ScreenerMethod
  {
    return self.high_;
  }

  auto low(this const auto& self) noexcept -> ScreenerMethod
  {
    return self.low_;
  }

  auto close(this const auto& self) noexcept -> ScreenerMethod
  {
    return self.close_;
  }

  auto k_period(this const auto& self) noexcept -> std::size_t
  {
    return self.k_period_;
  }

  void k_period(this auto& self, std::size_t k_period) noexcept
  {
    self.k_period_ = k_period;
  }

  auto k_smooth(this const auto& self) noexcept -> std::size_t
  {
    return self.k_smooth_;
  }

  void k_smooth(this auto& self, std::size_t k_smooth) noexcept
  {
    self.k_smooth_ = k_smooth;
  }

  auto d_period(this const auto& self) noexcept -> std::size_t
  {
    return self.d_period_;
  }

  void d_period(this auto& self, std::size_t d_period) noexcept
  {
    self.d_period_ = d_period;
  }

private:
  ScreenerMethod high_;
  ScreenerMethod low_;
  ScreenerMethod close_;

  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
};

} // namespace pludux::screener