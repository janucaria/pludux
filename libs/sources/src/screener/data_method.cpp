#include <algorithm>
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
 -> SubSeries<PolySeries<double>>
{
  if(!asset_data.contains(field_)) {
    throw std::runtime_error{"Field not found"};
  }
  return SubSeries<PolySeries<double>>{asset_data[field_],
                                       static_cast<std::ptrdiff_t>(offset_)};
}

auto DataMethod::field() const -> const std::string&
{
  return field_;
}

auto DataMethod::offset() const -> int
{
  return offset_;
}

} // namespace pludux::screener