#include <algorithm>
#include <iterator>
#include <vector>
#include <stdexcept>

#include <pludux/asset.hpp>

#include <pludux/screener/all_of_filter.hpp>

namespace pludux::screener {

AllOfFilter::AllOfFilter() = default;

AllOfFilter::AllOfFilter(std::vector<ScreenerFilter> filters)
  : filters_{std::move(filters)}
{
}

auto AllOfFilter::operator()(const Asset& asset) const -> bool
{
  for(const auto& filter : filters_) {
    if(!filter(asset)) {
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