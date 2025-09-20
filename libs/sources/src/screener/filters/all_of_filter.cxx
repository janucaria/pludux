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

  auto operator==(const AllOfFilter& other) const noexcept -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data) -> bool
  {
    return std::ranges::all_of(
     self.conditions_,
     [&asset_data](const auto& filter) { return filter(asset_data); });
  }

  auto conditions(this const auto& self) noexcept
   -> const std::vector<ScreenerFilter>&
  {
    return self.conditions_;
  }

private:
  std::vector<ScreenerFilter> conditions_;
};

} // namespace pludux::screener