module;

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:watchlist;

import :store_handle;

export namespace pludux::backtest {

class Watchlist {
public:
  Watchlist() = default;

  Watchlist(std::string name, std::vector<AssetStoreHandle> asset_handles = {})
  : name_{std::move(name)}
  {
    this->asset_handles(std::move(asset_handles));
  }

  auto operator==(const Watchlist&) const noexcept -> bool = default;

  auto name(this const Watchlist& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Watchlist& self, std::string value) noexcept
  {
    self.name_ = std::move(value);
  }

  auto asset_handles(this const Watchlist& self) noexcept
   -> const std::vector<AssetStoreHandle>&
  {
    return self.asset_handles_;
  }

  void asset_handles(this Watchlist& self, std::vector<AssetStoreHandle> value)
  {
    for(auto first = value.begin(); first != value.end(); ++first) {
      if(std::find(std::next(first), value.end(), *first) != value.end()) {
        throw std::invalid_argument{
         "Watchlist cannot contain duplicate assets"};
      }
    }
    self.asset_handles_ = std::move(value);
  }

  auto equivalent_rules(this const Watchlist& self,
                        const Watchlist& other) noexcept -> bool
  {
    return self.asset_handles_ == other.asset_handles_;
  }

private:
  std::string name_;
  std::vector<AssetStoreHandle> asset_handles_;
};

} // namespace pludux::backtest
