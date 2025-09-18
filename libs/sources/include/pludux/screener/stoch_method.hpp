#ifndef PLUDUX_PLUDUX_SCREENER_STOCH_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_STOCH_METHOD_HPP

#include <cstddef>

#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

class StochMethod {
public:
  StochMethod(ScreenerMethod high,
              ScreenerMethod low,
              ScreenerMethod close,
              std::size_t k_period,
              std::size_t k_smooth,
              std::size_t d_period);

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>;

  auto operator==(const StochMethod& other) const noexcept -> bool;

  auto high() const noexcept -> ScreenerMethod;

  auto low() const noexcept -> ScreenerMethod;

  auto close() const noexcept -> ScreenerMethod;

  auto k_period() const noexcept -> std::size_t;

  void k_period(std::size_t k_period) noexcept;

  auto k_smooth() const noexcept -> std::size_t;

  void k_smooth(std::size_t k_smooth) noexcept;

  auto d_period() const noexcept -> std::size_t;

  void d_period(std::size_t d_period) noexcept;

private:
  ScreenerMethod high_;
  ScreenerMethod low_;
  ScreenerMethod close_;

  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
};

} // namespace pludux::screener

#endif
