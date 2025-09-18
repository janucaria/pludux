module;

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

export module pludux:screener.all_of_filter;

import :asset_snapshot;
import :screener.screener_filter;

export namespace pludux::screener {

class AllOfFilter {
public:
  AllOfFilter() = default;

  explicit AllOfFilter(std::vector<ScreenerFilter> filters)
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