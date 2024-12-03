#include <vector>
#include <algorithm>

#include <pludux/asset.hpp>
#include <pludux/series.hpp>
#include <pludux/ta.hpp>

#include <pludux/screener/asset_data_provider.hpp>

namespace pludux::screener {

AssetDataProvider::AssetDataProvider(const Asset& asset)
: asset_{asset}
{
}

auto AssetDataProvider::open() const -> Series
{
  auto values = std::vector<double>{};
  values.reserve(asset_.quotes().size());
  std::transform(asset_.quotes().begin(), asset_.quotes().end(), std::back_inserter(values), [](const auto& quote) {
    return quote.open();
  });

  return Series{values};
}

auto AssetDataProvider::high() const -> Series
{
  auto values = std::vector<double>{};
  values.reserve(asset_.quotes().size());
  std::transform(asset_.quotes().begin(), asset_.quotes().end(), std::back_inserter(values), [](const auto& quote) {
    return quote.high();
  });

  return Series{values};
}

auto AssetDataProvider::low() const -> Series
{
  auto values = std::vector<double>{};
  values.reserve(asset_.quotes().size());
  std::transform(asset_.quotes().begin(), asset_.quotes().end(), std::back_inserter(values), [](const auto& quote) {
    return quote.low();
  });

  return Series{values};
}

auto AssetDataProvider::price() const -> Series
{
  auto values = std::vector<double>{};
  values.reserve(asset_.quotes().size());
  std::transform(asset_.quotes().begin(), asset_.quotes().end(), std::back_inserter(values), [](const auto& quote) {
    return quote.close();
  });

  return Series{values};
}

auto AssetDataProvider::volume() const -> Series
{
  auto values = std::vector<double>{};
  values.reserve(asset_.quotes().size());
  std::transform(asset_.quotes().begin(), asset_.quotes().end(), std::back_inserter(values), [](const auto& quote) {
    return quote.volume();
  });

  return Series{values};
}

} // namespace pludux::screener
