module;

#include <cmath>
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

  auto equivalent_with_nans_as_equal(this const Asset& self,
                                     const Asset& other) noexcept -> bool
  {
    if(self.name_ != other.name_) {
      return false;
    }

    return self.equivalent_rules(other);
  }

  auto name(this const Asset& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Asset& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

  auto history(this const Asset& self) noexcept -> const AssetHistory&
  {
    return self.asset_history_;
  }

  void history(this Asset& self, AssetHistory new_history) noexcept
  {
    self.asset_history_ = std::move(new_history);
  }

  auto field_resolver(this const Asset& self) noexcept
   -> const AssetQuoteFieldResolver&
  {
    return self.field_resolver_;
  }

  void field_resolver(this Asset& self,
                      AssetQuoteFieldResolver new_resolver) noexcept
  {
    self.field_resolver_ = std::move(new_resolver);
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

  auto equivalent_rules(this const Asset& self, const Asset& other) noexcept
   -> bool
  {
    if(self.field_resolver_ != other.field_resolver_) {
      return false;
    }

    const auto& self_field_data = self.history().field_data();
    const auto& other_field_data = other.history().field_data();

    if(self_field_data.size() != other_field_data.size()) {
      return false;
    }

    for(const auto& [field, self_series] : self_field_data) {
      const auto other_it = other_field_data.find(field);
      if(other_it == other_field_data.end()) {
        return false;
      }

      const auto& other_series = other_it->second;
      if(self_series.size() != other_series.size()) {
        return false;
      }

      for(std::size_t i = 0; i < self_series.size(); ++i) {
        const auto self_value = self_series[i];
        const auto other_value = other_series[i];

        const auto self_is_nan = std::isnan(self_value);
        const auto other_is_nan = std::isnan(other_value);

        if(self_is_nan && other_is_nan) {
          continue;
        }

        if(self_value != other_value) {
          return false;
        }
      }
    }

    return true;
  }

private:
  std::string name_;
  AssetHistory asset_history_;
  AssetQuoteFieldResolver field_resolver_;
};

} // namespace pludux::backtest
