#ifndef PLUDUX_PLUDUX_ASSET_DATA_PROVIDER_HPP
#define PLUDUX_PLUDUX_ASSET_DATA_PROVIDER_HPP

#include <vector>

#include <pludux/asset.hpp>
#include <pludux/series.hpp>

namespace pludux {

class AssetDataProvider {
public:
  explicit AssetDataProvider(const Asset& asset);

  auto timestamp() const noexcept -> const Series&;

  auto open() const noexcept -> const Series&;

  auto high() const noexcept -> const Series&;

  auto low() const noexcept -> const Series&;

  auto close() const noexcept -> const Series&;

  auto price() const noexcept -> const Series&;
  
  auto volume() const noexcept -> const Series&;

  auto changes() const -> Series;

  auto sma5() const -> Series;

  auto sma10() const -> Series;

  auto sma20() const -> Series;

  auto sma50() const -> Series;

  auto sma100() const -> Series;

  auto sma200() const -> Series;

  auto hma30() const -> Series;

  auto hma35() const -> Series;

  auto hma50() const -> Series;

  auto atr14() const -> Series;

  auto rsi14() const -> Series;

  auto rsi14_sma14() const -> Series;

  auto rsi14_bb14l2sd_upper() const -> Series;

  auto rsi14_bb14l2sd_lower() const -> Series;

private:
  const Asset& asset_;
};

} // namespace pludux::screener

#endif
