module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>

export module pludux.backtest:external_asset_data;

import pludux;
import :asset;

export namespace pludux::backtest {

class ExternalAssetData {
public:
  static auto mapped_field(const Asset& asset,
                           const std::string& field) noexcept
   -> const std::string&
  {
    const auto& resolver = asset.field_resolver();
    if(field == "OPEN")
      return resolver.open_field();
    if(field == "HIGH")
      return resolver.high_field();
    if(field == "LOW")
      return resolver.low_field();
    if(field == "CLOSE")
      return resolver.close_field();
    if(field == "VOLUME")
      return resolver.volume_field();
    return field;
  }

  static auto valid_timestamps(const Asset& asset) noexcept -> bool
  {
    const auto& columns = asset.history().field_data();
    const auto datetime = columns.find(asset.field_resolver().datetime_field());
    if(datetime == columns.end() || datetime->second.size() != asset.size())
      return false;
    const auto& timestamps = datetime->second.data();
    return std::ranges::all_of(
            timestamps, [](double value) { return std::isfinite(value); }) &&
           std::ranges::adjacent_find(timestamps, std::greater_equal<>{}) ==
            timestamps.end();
  }

  void add_field(const std::string& reference,
                 const std::string& field,
                 const Asset& asset)
  {
    const auto& columns = asset.history().field_data();
    const auto column = columns.find(mapped_field(asset, field));
    if(column == columns.end()) {
      throw std::invalid_argument{"Unknown external field '" + field + "'"};
    }
    auto source = references_.find(reference);
    if(source == references_.end()) {
      if(!valid_timestamps(asset)) {
        throw std::invalid_argument{
         "External candles require finite, strictly increasing timestamps"};
      }
      const auto& datetime =
       columns.at(asset.field_resolver().datetime_field());
      source =
       references_.try_emplace(reference, Reference{datetime, {}}).first;
    }
    source->second.fields.try_emplace(field, column->second);
  }

  auto value(const std::string& reference,
             const std::string& field,
             double timestamp) const noexcept -> double
  {
    constexpr auto missing = std::numeric_limits<double>::quiet_NaN();
    const auto source = references_.find(reference);
    if(source == references_.end() || !std::isfinite(timestamp))
      return missing;
    const auto& [datetime, fields] = source->second;
    const auto column = fields.find(field);
    if(column == fields.end())
      return missing;
    const auto& timestamps = datetime.data();
    const auto match = std::ranges::lower_bound(timestamps, timestamp);
    if(match == timestamps.end() || *match != timestamp)
      return missing;
    return column->second[static_cast<std::size_t>(match - timestamps.begin())];
  }

private:
  struct Reference {
    AssetData datetime;
    std::map<std::string, AssetData> fields;
  };

  std::map<std::string, Reference> references_;
};

} // namespace pludux::backtest
