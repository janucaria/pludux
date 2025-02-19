#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <pludux/asset_snapshot.hpp>

#include <pludux/screener/any_of_filter.hpp>

namespace pludux::screener {

AnyOfFilter::AnyOfFilter() = default;

AnyOfFilter::AnyOfFilter(std::vector<ScreenerFilter> filters)
: conditions_{std::move(filters)}
{
}

auto AnyOfFilter::operator()(AssetSnapshot asset_data) const -> bool
{
  return std::any_of(
   std::cbegin(conditions_),
   std::cend(conditions_),
   [&asset_data](const auto& filter) { return filter(asset_data); });
}

auto AnyOfFilter::filters() const -> const std::vector<ScreenerFilter>&
{
  return conditions_;
}

} // namespace pludux::screener