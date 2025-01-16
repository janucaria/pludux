#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset.hpp>

#include <pludux/screener/value_method.hpp>

namespace pludux::screener {

ValueMethod::ValueMethod(double value)
: value_{value}
{
}

auto ValueMethod::operator()(const AssetDataProvider& asset_data) const -> PolySeries<double>
{
  return RepeatSeries{value_, asset_data.price().size()};
}

auto ValueMethod::value() const -> double
{
  return value_;
}

} // namespace pludux::screener