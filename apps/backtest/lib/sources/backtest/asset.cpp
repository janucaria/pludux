#include <utility>

#include <pludux/backtest/asset.hpp>

namespace pludux::backtest {

Asset::Asset(std::string name, AssetHistory asset_history)
: name_{std::move(name)}
, asset_history_{std::move(asset_history)}
{
  asset_history_.datetime_key("Date");
}

auto Asset::name() const noexcept -> const std::string&
{
  return name_;
}

auto Asset::history() const noexcept -> const AssetHistory&
{
  return asset_history_;
}

} // namespace pludux::backtest
