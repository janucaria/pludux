#ifndef PLUDUX_PLUDUX_SCREENER_ASSET_DATA_PROVIDER_HPP
#define PLUDUX_PLUDUX_SCREENER_ASSET_DATA_PROVIDER_HPP

#include <vector>

#include <pludux/asset.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class AssetDataProvider {
public:
  explicit AssetDataProvider(const Asset& asset);

  auto open() const -> Series;

  auto high() const -> Series;

  auto low() const -> Series;

  auto price() const -> Series;
  
  auto volume() const -> Series;

private:
  const Asset& asset_;
};

} // namespace pludux::screener

#endif
