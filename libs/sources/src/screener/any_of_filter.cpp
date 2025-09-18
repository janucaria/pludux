#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>


#include <pludux/screener/any_of_filter.hpp>

namespace pludux::screener {

AnyOfFilter::AnyOfFilter() = default;

AnyOfFilter::AnyOfFilter(std::vector<ScreenerFilter> conditions)
: conditions_{std::move(conditions)}
{
}

auto AnyOfFilter::operator()(AssetSnapshot asset_data) const -> bool
{
  return std::any_of(
   std::cbegin(conditions_),
   std::cend(conditions_),
   [&asset_data](const auto& filter) { return filter(asset_data); });
}

auto AnyOfFilter::operator==(const AnyOfFilter& other) const noexcept -> bool = default;

auto AnyOfFilter::conditions() const -> const std::vector<ScreenerFilter>&
{
  return conditions_;
}

} // namespace pludux::screener