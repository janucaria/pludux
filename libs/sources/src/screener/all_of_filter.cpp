#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/all_of_filter.hpp>

namespace pludux::screener {

AllOfFilter::AllOfFilter() = default;

AllOfFilter::AllOfFilter(std::vector<ScreenerFilter> filters)
: conditions_{std::move(filters)}
{
}

auto AllOfFilter::operator()(AssetSnapshot asset_data) const -> bool
{
  return std::all_of(
   std::cbegin(conditions_),
   std::cend(conditions_),
   [&asset_data](const auto& filter) { return filter(asset_data); });
}

auto AllOfFilter::filters() const -> const std::vector<ScreenerFilter>&
{
  return conditions_;
}

} // namespace pludux::screener