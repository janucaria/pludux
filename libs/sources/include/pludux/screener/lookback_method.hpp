#ifndef PLUDUX_PLUDUX_SCREENER_LOOKBACK_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_LOOKBACK_METHOD_HPP

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

class LookbackMethod {
public:
  explicit LookbackMethod(ScreenerMethod source, std::size_t period);

  auto operator==(const LookbackMethod& other) const noexcept -> bool;

  auto operator()(this const LookbackMethod self, AssetSnapshot asset_data)
   -> PolySeries<double>;

  auto source(this const LookbackMethod self) noexcept -> ScreenerMethod;

  void source(this LookbackMethod self, ScreenerMethod source) noexcept;

  auto period(this const LookbackMethod self) noexcept -> std::size_t;

  void period(this LookbackMethod self, std::size_t period) noexcept;

private:
  ScreenerMethod source_;
  std::size_t period_;
};

} // namespace pludux::screener

#endif
