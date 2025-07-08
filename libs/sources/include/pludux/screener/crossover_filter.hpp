#ifndef PLUDUX_PLUDUX_SCREENER_CROSSOVER_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_CROSSOVER_FILTER_HPP

import pludux.asset_snapshot;

#include <vector>

#include <pludux/screener/screener_filter.hpp>
#include <pludux/screener/screener_method.hpp>

namespace pludux::screener {

class CrossoverFilter {
public:
  explicit CrossoverFilter(ScreenerMethod signal, ScreenerMethod reference);

  auto operator()(AssetSnapshot asset_data) const -> bool;

  auto operator==(const CrossoverFilter& other) const noexcept -> bool;

  auto signal() const noexcept -> const ScreenerMethod&;

  auto reference() const noexcept -> const ScreenerMethod&;

private:
  ScreenerMethod signal_;
  ScreenerMethod reference_;
};

} // namespace pludux::screener

#endif
