module;

#include <algorithm>
#include <iterator>
#include <vector>

export module pludux.screener.any_of_filter;

import pludux.asset_snapshot;
import pludux.screener.screener_filter;

namespace pludux::screener {

export class AnyOfFilter {
public:
  AnyOfFilter() = default;

  AnyOfFilter(std::vector<ScreenerFilter> conditions)
  : conditions_{std::move(conditions)}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> bool
  {
    return std::any_of(
     std::cbegin(conditions_),
     std::cend(conditions_),
     [&asset_data](const auto& filter) { return filter(asset_data); });
  }

  auto operator==(const AnyOfFilter& other) const noexcept -> bool = default;

  auto conditions() const -> const std::vector<ScreenerFilter>&
  {
    return conditions_;
  }

private:
  std::vector<ScreenerFilter> conditions_;
};

} // namespace pludux::screener
