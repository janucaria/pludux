#include <algorithm>
#include <iterator>
#include <vector>
#include <stdexcept>

#include <pludux/asset.hpp>

#include <pludux/screener/field_method.hpp>

namespace pludux::screener {

FieldMethod::FieldMethod(std::string field, int offset)
  : field_{std::move(field)}
  , offset_{offset}
{
}

auto FieldMethod::run_all(const AssetDataProvider& asset_data) const -> Series
{
  if(field_ == "close") {
    return asset_data.price().subseries(offset_);
  }

  if(field_ == "open") {
    return asset_data.open().subseries(offset_);
  }

  if(field_ == "high") {
    return asset_data.high().subseries(offset_);
  }

  if(field_ == "low") {
    return asset_data.low().subseries(offset_);
  }

  if(field_ == "volume") {
    return asset_data.volume().subseries(offset_);
  }

  throw std::runtime_error("Field not found");
}

auto FieldMethod::run_one(const AssetDataProvider& asset_data) const -> double
{
  const auto series = run_all(asset_data);
  return series[0];
}

auto FieldMethod::field() const -> const std::string&
{
  return field_;
}

auto FieldMethod::offset() const -> int
{
  return offset_;
}

} // namespace pludux::screener