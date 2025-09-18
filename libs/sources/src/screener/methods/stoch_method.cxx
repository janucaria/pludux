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

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    const auto high_series = high_(asset_data);
    const auto low_series = low_(asset_data);
    const auto close_series = close_(asset_data);

    const auto stoch = StochSeries{
     high_series, low_series, close_series, k_period_, k_smooth_, d_period_};

    return stoch;
  }

  auto operator==(const StochMethod& other) const noexcept -> bool = default;

  auto high() const noexcept -> ScreenerMethod
  {
    return high_;
  }

  auto low() const noexcept -> ScreenerMethod
  {
    return low_;
  }

  auto close() const noexcept -> ScreenerMethod
  {
    return close_;
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
  ScreenerMethod high_;
  ScreenerMethod low_;
  ScreenerMethod close_;

  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
};

} // namespace pludux::screener