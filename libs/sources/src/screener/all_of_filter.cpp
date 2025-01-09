#include <algorithm>
#include <iterator>
#include <vector>
#include <stdexcept>

#include <pludux/asset.hpp>

#include <pludux/screener/all_of_filter.hpp>
#include <pludux/asset_data_provider.hpp>

namespace pludux::screener {

AllOfFilter::AllOfFilter() = default;

AllOfFilter::AllOfFilter(std::vector<ScreenerFilter> filters)
  : filters_{std::move(filters)}
{
}

auto AllOfFilter::operator()(const AssetDataProvider& asset_data) const -> bool
{
  for(const auto& filter : filters_) {
    if(!filter(asset_data)) {
      return false;
    }
  }
  return true;
}

auto AllOfFilter::filters() const -> const std::vector<ScreenerFilter>&
{
  return filters_;
}

} // namespace pludux::screener