#include <algorithm>
#include <format>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <pludux/asset_snapshot.hpp>

#include <pludux/screener/data_method.hpp>

namespace pludux::screener {

DataMethod::DataMethod(std::string field, std::size_t offset)
: field_{std::move(field)}
, offset_{offset}
{
}

auto DataMethod::operator()(AssetSnapshot asset_data) const
 -> PolySeries<double>
{
  if(!asset_data.contains(field_)) {
    return LookbackSeries{
     PolySeries<double>{RepeatSeries{std::numeric_limits<double>::quiet_NaN(),
                                     asset_data.size()}},
     static_cast<std::ptrdiff_t>(offset_)};
  }
  return LookbackSeries<PolySeries<double>>{
   asset_data.get_values(field_), static_cast<std::ptrdiff_t>(offset_)};
}

auto DataMethod::operator==(const DataMethod& other) const noexcept
 -> bool = default;

auto DataMethod::field() const -> const std::string&
{
  return field_;
}

auto DataMethod::offset() const -> int
{
  return offset_;
}

} // namespace pludux::screener