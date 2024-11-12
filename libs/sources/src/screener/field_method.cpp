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

auto FieldMethod::run_all(const Asset& asset) const -> Series
{
  if(field_ == "close") {
    auto closes = std::vector<double>{};
    closes.reserve(asset.quotes().size());
    for(const auto& quote : asset.quotes()) {
      closes.push_back(quote.close());
    }
    const auto series = Series{std::move(closes)};
    return series.subseries(offset_);
  }

  if(field_ == "open") {
    auto opens = std::vector<double>{};
    opens.reserve(asset.quotes().size());
    for(const auto& quote : asset.quotes()) {
      opens.push_back(quote.open());
    }
    const auto series = Series{std::move(opens)};
    return series.subseries(offset_);
  }

  if(field_ == "high") {
    auto highs = std::vector<double>{};
    highs.reserve(asset.quotes().size());
    for(const auto& quote : asset.quotes()) {
      highs.push_back(quote.high());
    }
    const auto series = Series{std::move(highs)};
    return series.subseries(offset_);
  }

  if(field_ == "low") {
    auto lows = std::vector<double>{};
    lows.reserve(asset.quotes().size());
    for(const auto& quote : asset.quotes()) {
      lows.push_back(quote.low());
    }
    const auto series = Series{std::move(lows)};
    return series.subseries(offset_);
  }

  if(field_ == "volume") {
    auto volumes = std::vector<double>{};
    volumes.reserve(asset.quotes().size());
    for(const auto& quote : asset.quotes()) {
      volumes.push_back(quote.volume());
    }
    const auto series = Series{std::move(volumes)};
    return series.subseries(offset_);
  }

  throw std::runtime_error("Field not found");
}

auto FieldMethod::run_one(const Asset& asset) const -> double
{
  const auto series = run_all(asset);
  return series[0];
}

} // namespace pludux::screener