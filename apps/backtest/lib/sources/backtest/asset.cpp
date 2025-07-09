#include <utility>

#include <pludux/backtest/asset.hpp>

namespace pludux::backtest {

Asset::Asset(std::string name, AssetHistory asset_history)
: name_{std::move(name)}
, asset_history_{std::move(asset_history)}
{
}

auto Asset::name() const noexcept -> const std::string&
{
  return name_;
}

auto Asset::history() const noexcept -> const AssetHistory&
{
  return asset_history_;
}

auto Asset::get_snapshot(std::size_t index) const noexcept -> AssetSnapshot
{
  return AssetSnapshot{index, asset_history_};
}

auto Asset::size() const noexcept -> std::size_t
{
  return asset_history_.size();
}

} // namespace pludux::backtest
