#ifndef PLUDUX_PLUDUX_ASSET_DATA_PROVIDER_HPP
#define PLUDUX_PLUDUX_ASSET_DATA_PROVIDER_HPP

#include <vector>

#include <pludux/asset.hpp>
#include <pludux/series.hpp>
#include <pludux/ta.hpp>

namespace pludux {

class AssetDataProvider {
public:
  explicit AssetDataProvider(const Asset& asset);

  auto timestamp() const noexcept
  {
    return asset_.quotes().timestamps();
  }

  auto open() const noexcept
  {
    return asset_.quotes().opens();
  }

  auto high() const noexcept
  {
    return asset_.quotes().highs();
  }

  auto low() const noexcept
  {
    return asset_.quotes().lows();
  }

  auto close() const noexcept
  {
    return asset_.quotes().closes();
  }

  auto price() const noexcept
  {
    return close();
  }

  auto volume() const noexcept

  {
    return asset_.quotes().volumes();
  }

  auto changes() const
  {
    return ta::changes(price());
  }

  auto sma5() const
  {
    return ta::sma(price(), 5);
  }

  auto sma10() const
  {
    return ta::sma(price(), 10);
  }

  auto sma20() const
  {
    return ta::sma(price(), 20);
  }

  auto sma50() const
  {
    return ta::sma(price(), 50);
  }

  auto sma100() const
  {
    return ta::sma(price(), 100);
  }

  auto sma200() const
  {
    return ta::sma(price(), 200);
  }

  auto hma30() const
  {
    return ta::hma(price(), 30);
  }

  auto hma35() const
  {
    return ta::hma(price(), 35);
  }

  auto hma50() const
  {
    return ta::hma(price(), 50);
  }

  auto atr14() const
  {
    return ta::atr(high(), low(), close(), 14);
  }

  auto rsi14() const
  {
    return ta::rsi(price(), 14);
  }

  auto rsi14_sma14() const
  {
    return ta::sma(rsi14(), 14);
  }

  auto rsi14_bb14l2sd_upper() const
  {
    return ta::bb(rsi14_sma14(), 14, 2).upper_band();
  }

  auto rsi14_bb14l2sd_lower() const
  {
    return ta::bb(rsi14_sma14(), 14, 2).lower_band();
  }

private:
  const Asset& asset_;
};

} // namespace pludux

#endif
