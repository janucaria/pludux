#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset_snapshot.hpp>

#include <pludux/screener/value_method.hpp>

namespace pludux::screener {

ValueMethod::ValueMethod(double value)
: value_{value}
{
}

auto ValueMethod::operator()(AssetSnapshot asset_data) const
 -> PolySeries<double>
{
  return RepeatSeries{value_, asset_data.size()};
}

auto ValueMethod::operator==(const ValueMethod& other) const noexcept
 -> bool = default;

auto ValueMethod::value() const -> double
{
  return value_;
}

} // namespace pludux::screener