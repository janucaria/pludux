#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/screener.hpp>

namespace pludux::screener {

GreaterEqualFilter::GreaterEqualFilter(ScreenerMethod target,
                                       ScreenerMethod threshold)
: target_{std::move(target)}
, threshold_{std::move(threshold)}
{
}

auto GreaterEqualFilter::operator()(const Asset& asset) const -> bool
{
  const auto target_result = target_.run_one(asset);
  const auto threshold_result = threshold_.run_one(asset);

  return target_result >= threshold_result;
}

} // namespace pludux::screener