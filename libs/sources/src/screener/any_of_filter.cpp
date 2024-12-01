#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <pludux/asset.hpp>

#include <pludux/screener/any_of_filter.hpp>

namespace pludux::screener {

AnyOfFilter::AnyOfFilter() = default;

AnyOfFilter::AnyOfFilter(std::vector<ScreenerFilter> filters)
: filters_{std::move(filters)}
{
}

auto AnyOfFilter::operator()(const Asset& asset) const -> bool
{
  for(const auto& filter : filters_) {
    if(filter(asset)) {
      return true;
    }
  }
  return false;
}

auto AnyOfFilter::filters() const -> const std::vector<ScreenerFilter>&
{
  return filters_;
}

} // namespace pludux::screener