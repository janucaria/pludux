#include <algorithm>
#include <iterator>
#include <vector>
#include <stdexcept>

#include <pludux/asset.hpp>

#include <pludux/screener/data_method.hpp>

namespace pludux::screener {

DataMethod::DataMethod(std::string field, int offset)
  : field_{std::move(field)}
  , offset_{offset}
{
}

auto DataMethod::run_all(const AssetDataProvider& asset_data) const -> Series
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

  if(field_ == "sma5") {
    return asset_data.sma5().subseries(offset_);
  }

  if(field_ == "sma10") {
    return asset_data.sma10().subseries(offset_);
  }

  if(field_ == "sma20") {
    return asset_data.sma20().subseries(offset_);
  }

  if(field_ == "sma50") {
    return asset_data.sma50().subseries(offset_);
  }

  if(field_ == "sma100") {
    return asset_data.sma100().subseries(offset_);
  }

  if(field_ == "sma200") {
    return asset_data.sma200().subseries(offset_);
  }

  if(field_ == "rsi14") {
    return asset_data.rsi14().subseries(offset_);
  }

  if(field_ == "rsi14_sma14") {
    return asset_data.rsi14_sma14().subseries(offset_);
  }

  if(field_ == "rsi14_bb14l2sd_upper") {
    return asset_data.rsi14_bb14l2sd_upper().subseries(offset_);
  }

  if(field_ == "rsi14_bb14l2sd_lower") {
    return asset_data.rsi14_bb14l2sd_lower().subseries(offset_);
  }

  throw std::runtime_error("Field not found");
}

auto DataMethod::run_one(const AssetDataProvider& asset_data) const -> double
{
  const auto series = run_all(asset_data);
  return series[0];
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