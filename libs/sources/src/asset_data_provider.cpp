#include <algorithm>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/series.hpp>
#include <pludux/ta.hpp>

#include <pludux/asset_data_provider.hpp>

namespace pludux {

AssetDataProvider::AssetDataProvider(const Asset& asset)
: asset_{asset}
{
}

auto AssetDataProvider::timestamp() const noexcept -> const Series&
{
  return asset_.quotes().timestamps();
}

auto AssetDataProvider::open() const noexcept -> const Series&
{
  return asset_.quotes().opens();
}

auto AssetDataProvider::high() const noexcept -> const Series&
{
  return asset_.quotes().highs();
}

auto AssetDataProvider::low() const noexcept -> const Series&
{
  return asset_.quotes().lows();
}

auto AssetDataProvider::close() const noexcept -> const Series&
{
  return asset_.quotes().closes();
}

auto AssetDataProvider::price() const noexcept -> const Series&
{
  return close();
}

auto AssetDataProvider::volume() const noexcept -> const Series&
{
  return asset_.quotes().volumes();
}

auto AssetDataProvider::changes() const -> Series
{
  return ta::changes(price());
}

auto AssetDataProvider::sma5() const -> Series
{
  return ta::sma(price(), 5);
}

auto AssetDataProvider::sma10() const -> Series
{
  return ta::sma(price(), 10);
}

auto AssetDataProvider::sma20() const -> Series
{
  return ta::sma(price(), 20);
}

auto AssetDataProvider::sma50() const -> Series
{
  return ta::sma(price(), 50);
}

auto AssetDataProvider::sma100() const -> Series
{
  return ta::sma(price(), 100);
}

auto AssetDataProvider::sma200() const -> Series
{
  return ta::sma(price(), 200);
}

auto AssetDataProvider::hma30() const -> Series
{
  return ta::hma(price(), 30);
}

auto AssetDataProvider::hma35() const -> Series
{
  return ta::hma(price(), 35);
}

auto AssetDataProvider::hma50() const -> Series
{
  return ta::hma(price(), 50);
}

auto AssetDataProvider::atr14() const -> Series
{
  return ta::atr(high(), low(), close(), 14);
}

auto AssetDataProvider::rsi14() const -> Series
{
  return ta::rsi(price(), 14);
}

auto AssetDataProvider::rsi14_sma14() const -> Series
{
  return ta::sma(rsi14(), 14);
}

auto AssetDataProvider::rsi14_bb14l2sd_upper() const -> Series
{
  const auto rsi14_sma14 = rsi14();
  const auto [bb_middle, bb_upper, bb_lower] = ta::bb(rsi14_sma14, 14, 2);

  return bb_upper;
}

auto AssetDataProvider::rsi14_bb14l2sd_lower() const -> Series
{
  const auto rsi14_sma14 = rsi14();
  const auto [bb_middle, bb_upper, bb_lower] = ta::bb(rsi14_sma14, 14, 2);

  return bb_lower;
}

} // namespace pludux
