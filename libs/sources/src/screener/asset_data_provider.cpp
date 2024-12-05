#include <algorithm>
#include <vector>

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
  std::transform(asset_.quotes().begin(),
                 asset_.quotes().end(),
                 std::back_inserter(values),
                 [](const auto& quote) { return quote.open(); });

  return Series{values};
}

auto AssetDataProvider::high() const -> Series
{
  auto values = std::vector<double>{};
  values.reserve(asset_.quotes().size());
  std::transform(asset_.quotes().begin(),
                 asset_.quotes().end(),
                 std::back_inserter(values),
                 [](const auto& quote) { return quote.high(); });

  return Series{values};
}

auto AssetDataProvider::low() const -> Series
{
  auto values = std::vector<double>{};
  values.reserve(asset_.quotes().size());
  std::transform(asset_.quotes().begin(),
                 asset_.quotes().end(),
                 std::back_inserter(values),
                 [](const auto& quote) { return quote.low(); });

  return Series{values};
}

auto AssetDataProvider::price() const -> Series
{
  auto values = std::vector<double>{};
  values.reserve(asset_.quotes().size());
  std::transform(asset_.quotes().begin(),
                 asset_.quotes().end(),
                 std::back_inserter(values),
                 [](const auto& quote) { return quote.close(); });

  return Series{values};
}

auto AssetDataProvider::volume() const -> Series
{
  auto values = std::vector<double>{};
  values.reserve(asset_.quotes().size());
  std::transform(asset_.quotes().begin(),
                 asset_.quotes().end(),
                 std::back_inserter(values),
                 [](const auto& quote) { return quote.volume(); });

  return Series{values};
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
  const auto[ bb_middle, bb_upper, bb_lower ] = ta::bb(rsi14_sma14, 14, 2);

  return bb_upper;
}

auto AssetDataProvider::rsi14_bb14l2sd_lower() const -> Series
{
  const auto rsi14_sma14 = rsi14();
  const auto[ bb_middle, bb_upper, bb_lower ] = ta::bb(rsi14_sma14, 14, 2);

  return bb_lower;
}


} // namespace pludux::screener
