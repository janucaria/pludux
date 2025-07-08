#ifndef PLUDUX_PLUDUX_SCREENER_STOCH_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_STOCH_METHOD_HPP

import pludux.asset_snapshot;
import pludux.series;

#include <cstddef>

#include <pludux/screener/screener_method.hpp>

namespace pludux::screener {

class StochMethod {
public:
  StochMethod(StochOutput output,
              ScreenerMethod high,
              ScreenerMethod low,
              ScreenerMethod close,
              std::size_t k_period,
              std::size_t k_smooth,
              std::size_t d_period,
              std::size_t offset = 0);

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>;

  auto operator==(const StochMethod& other) const noexcept -> bool;

  auto high() const noexcept -> ScreenerMethod;

  auto low() const noexcept -> ScreenerMethod;

  auto close() const noexcept -> ScreenerMethod;

  auto offset() const noexcept -> std::size_t;

  void offset(std::size_t offset) noexcept;

  auto output() const noexcept -> StochOutput;

  void output(StochOutput output) noexcept;

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
  StochOutput output_;
  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
  std::size_t offset_;
};

} // namespace pludux::screener

#endif
