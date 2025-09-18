#ifndef PLUDUX_PLUDUX_SCREENER_ANY_OF_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_ANY_OF_FILTER_HPP

#include <vector>

import pludux.asset_snapshot;
import pludux.screener.screener_filter;

namespace pludux::screener {

class AnyOfFilter {
public:
  AnyOfFilter();

  explicit AnyOfFilter(std::vector<ScreenerFilter> conditions);

  auto operator()(AssetSnapshot asset_data) const -> bool;

  auto operator==(const AnyOfFilter& other) const noexcept-> bool;

  auto conditions() const -> const std::vector<ScreenerFilter>&;

private:
  std::vector<ScreenerFilter> conditions_;
};

} // namespace pludux::screener

#endif
