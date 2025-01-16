#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <pludux/asset.hpp>

#include <pludux/screener/data_method.hpp>

namespace pludux::screener {

DataMethod::DataMethod(std::string field, std::size_t offset)
: field_{std::move(field)}
, offset_{offset}
{
}

auto DataMethod::operator()(const AssetDataProvider& asset_data) const -> PolySeries<double>
{
  if(field_ == "close") {
    return SubSeries{asset_data.price(), offset_};
  }

  if(field_ == "open") {
    return SubSeries{asset_data.open(), offset_};
  }

  if(field_ == "high") {
    return SubSeries{asset_data.high(), offset_};
  }

  if(field_ == "low") {
    return SubSeries{asset_data.low(), offset_};
  }

  if(field_ == "volume") {
    return SubSeries{asset_data.volume(), offset_};
  }

  if(field_ == "sma5") {
    return SubSeries{asset_data.sma5(), offset_};
  }

  if(field_ == "sma10") {
    return SubSeries{asset_data.sma10(), offset_};
  }

  if(field_ == "sma20") {
    return SubSeries{asset_data.sma20(), offset_};
  }

  if(field_ == "sma50") {
    return SubSeries{asset_data.sma50(), offset_};
  }

  if(field_ == "sma100") {
    return SubSeries{asset_data.sma100(), offset_};
  }

  if(field_ == "sma200") {
    return SubSeries{asset_data.sma200(), offset_};
  }

  if(field_ == "hma30") {
    return SubSeries{asset_data.hma30(), offset_};
  }

  if(field_ == "hma35") {
    return SubSeries{asset_data.hma35(), offset_};
  }

  if(field_ == "hma50") {
    return SubSeries{asset_data.hma50(), offset_};
  }

  if(field_ == "atr14") {
    return SubSeries{asset_data.atr14(), offset_};
  }

  if(field_ == "rsi14") {
    return SubSeries{asset_data.rsi14(), offset_};
  }

  if(field_ == "rsi14_sma14") {
    return SubSeries{asset_data.rsi14_sma14(), offset_};
  }

  if(field_ == "rsi14_bb14l2sd_upper") {
    return SubSeries{asset_data.rsi14_bb14l2sd_upper(), offset_};
  }

  if(field_ == "rsi14_bb14l2sd_lower") {
    return SubSeries{asset_data.rsi14_bb14l2sd_lower(), offset_};
  }

  throw std::runtime_error("Field not found");
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