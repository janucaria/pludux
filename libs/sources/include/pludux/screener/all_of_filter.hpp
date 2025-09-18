#ifndef PLUDUX_PLUDUX_SCREENER_All_OF_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_All_OF_FILTER_HPP

#include <vector>

import pludux.asset_snapshot;
import pludux.screener.screener_filter;

namespace pludux::screener {

class AllOfFilter {
public:
  AllOfFilter();

  explicit AllOfFilter(std::vector<ScreenerFilter> filters);

  auto operator()(AssetSnapshot asset_data) const -> bool;

  auto operator==(const AllOfFilter& other) const noexcept-> bool;

  auto conditions() const -> const std::vector<ScreenerFilter>&;

private:
  std::vector<ScreenerFilter> conditions_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_All_OF_FILTER_HPP
