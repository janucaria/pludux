module;

#include <string>
#include <utility>

export module pludux.backtest:asset;

import pludux.asset_snapshot;

export namespace pludux::backtest {

class Asset {
public:
  Asset(std::string name, AssetHistory asset_history);

  auto name() const noexcept -> const std::string&;

  auto history() const noexcept -> const AssetHistory&;

  auto get_snapshot(std::size_t index = 0) const noexcept -> AssetSnapshot;

  auto size() const noexcept -> std::size_t;

private:
  std::string name_;
  AssetHistory asset_history_;
};

// ------------------------------------------------------------------------

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
