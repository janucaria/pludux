#ifndef PLUDUX_PLUDUX_SCREENER_CROSSUNDER_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_CROSSUNDER_FILTER_HPP

#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_filter.hpp>
#include <pludux/screener/screener_method.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

class CrossunderFilter {
public:
  explicit CrossunderFilter(ScreenerMethod signal, ScreenerMethod reference);

  auto operator()(AssetSnapshot asset_data) const -> bool;

  auto operator==(const CrossunderFilter& other) const noexcept -> bool;

  auto signal() const noexcept -> const ScreenerMethod&;

  auto reference() const noexcept -> const ScreenerMethod&;

private:
  ScreenerMethod signal_;
  ScreenerMethod reference_;
};

} // namespace pludux::screener

#endif
