module;

#include <string>
#include <utility>

export module pludux.backtest:asset;

import pludux;

export namespace pludux::backtest {

class Asset {
public:
  Asset()
  : Asset{"", AssetHistory{}}
  {
  }

  Asset(std::string name, AssetHistory asset_history)
  : name_{std::move(name)}
  , asset_history_{std::move(asset_history)}
  {
  }

  auto name(this const Asset& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  auto history(this const Asset& self) noexcept -> const AssetHistory&
  {
    return self.asset_history_;
  }

  auto get_snapshot(this const Asset& self, std::size_t index) noexcept
   -> AssetSnapshot
  {
    return AssetSnapshot{index, self.history()};
  }

  auto size(this const Asset& self) noexcept -> std::size_t
  {
    return self.history().size();
  }

private:
  std::string name_;
  AssetHistory asset_history_;
};

} // namespace pludux::backtest
