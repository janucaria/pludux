#include <algorithm>
#include <format>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <pludux/asset_snapshot.hpp>

#include <pludux/screener/data_method.hpp>

namespace pludux::screener {

DataMethod::DataMethod(std::string field)
: field_{std::move(field)}
{
}

auto DataMethod::operator()(AssetSnapshot asset_data) const
 -> PolySeries<double>
{
  if(!asset_data.contains(field_)) {
    return RepeatSeries{std::numeric_limits<double>::quiet_NaN(),
                        asset_data.size()};
  }
  return asset_data.get_values(field_);
}

auto DataMethod::operator==(const DataMethod& other) const noexcept
 -> bool = default;

auto DataMethod::field() const -> const std::string&
{
  return field_;
}

} // namespace pludux::screener