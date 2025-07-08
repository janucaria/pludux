module;

#include <algorithm>
#include <iterator>
#include <vector>

export module pludux.screener.all_of_filter;

import pludux.asset_snapshot;
import pludux.screener.screener_filter;

namespace pludux::screener {

export class AllOfFilter {
public:
  AllOfFilter() = default;

  AllOfFilter(std::vector<ScreenerFilter> filters)
  : conditions_{std::move(filters)}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> bool
  {
    return std::all_of(
     std::cbegin(conditions_),
     std::cend(conditions_),
     [&asset_data](const auto& filter) { return filter(asset_data); });
  }

  auto operator==(const AllOfFilter& other) const noexcept -> bool = default;

  auto conditions() const -> const std::vector<ScreenerFilter>&
  {
    return conditions_;
  }

private:
  std::vector<ScreenerFilter> conditions_;
};

} // namespace pludux::screener
