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

  auto sma5() const -> Series;

  auto sma10() const -> Series;

  auto sma20() const -> Series;

  auto sma50() const -> Series;

  auto sma100() const -> Series;

  auto sma200() const -> Series;

  auto rsi14() const -> Series;

  auto rsi14_sma14() const -> Series;

  auto rsi14_bb14l2sd_upper() const -> Series;

  auto rsi14_bb14l2sd_lower() const -> Series;

private:
  const Asset& asset_;
};

} // namespace pludux::screener

#endif
