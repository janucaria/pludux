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

  Asset(std::string name)
  : Asset{std::move(name), AssetHistory{}}
  {
  }

  Asset(std::string name, AssetHistory asset_history)
  : Asset{std::move(name), std::move(asset_history), AssetQuoteFieldResolver{}}
  {
  }

  Asset(std::string name,
        AssetHistory asset_history,
        AssetQuoteFieldResolver field_resolver)
  : name_{std::move(name)}
  , asset_history_{std::move(asset_history)}
  , field_resolver_{std::move(field_resolver)}
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

  auto field_resolver(this const Asset& self) noexcept
   -> const AssetQuoteFieldResolver&
  {
    return self.field_resolver_;
  }

  auto get_snapshot(this const Asset& self, std::size_t offset) noexcept
   -> AssetSnapshot
  {
    return AssetSnapshot{offset, self.history(), self.field_resolver()};
  }

  auto size(this const Asset& self) noexcept -> std::size_t
  {
    return self.history().size();
  }

private:
  std::string name_;
  AssetHistory asset_history_;
  AssetQuoteFieldResolver field_resolver_;
};

} // namespace pludux::backtest
