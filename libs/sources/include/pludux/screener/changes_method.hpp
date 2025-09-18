#ifndef PLUDUX_PLUDUX_SCREENER_CHANGES_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_CHANGES_METHOD_HPP

#include <cstddef>

#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

class ChangesMethod {
public:
  ChangesMethod();

  explicit ChangesMethod(ScreenerMethod input);

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>;

  auto operator==(const ChangesMethod& other) const noexcept -> bool;

  auto input() const noexcept -> const ScreenerMethod&;

  void input(ScreenerMethod input) noexcept;

private:
  ScreenerMethod input_;
};

} // namespace pludux::screener

#endif
