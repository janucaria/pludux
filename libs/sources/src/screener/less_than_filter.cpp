#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <pludux/asset.hpp>

#include <pludux/screener/less_than_filter.hpp>

namespace pludux::screener {

LessThanFilter::LessThanFilter(ScreenerMethod target, ScreenerMethod threshold)
: target_{std::move(target)}
, threshold_{std::move(threshold)}
{
}

auto LessThanFilter::operator()(const Asset& asset) const -> bool
{
  const auto target_result = target_.run_one(asset);
  const auto threshold_reseult = threshold_.run_one(asset);

  return target_result < threshold_reseult;
}

} // namespace pludux::screener