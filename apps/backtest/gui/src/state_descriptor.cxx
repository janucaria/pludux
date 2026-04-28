module;

#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <queue>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

export module pludux.apps.backtest:state_descriptor;

import pludux.backtest;

export namespace pludux::apps {

class StateDescriptor {
public:
  StateDescriptor() = default;

  StateDescriptor(
   backtest::StoreHandle<backtest::Backtest> selected_backtest_handle,
   std::vector<backtest::StoreHandle<backtest::Backtest>> backtest_handles,
   std::vector<backtest::StoreHandle<backtest::Asset>> asset_handles,
   std::vector<backtest::StoreHandle<backtest::Strategy>> strategy_handles,
   std::vector<backtest::StoreHandle<backtest::Market>> market_handles,
   std::vector<backtest::StoreHandle<backtest::Broker>> broker_handles,
   std::vector<backtest::StoreHandle<backtest::Profile>> profile_handles)
  : selected_backtest_handle_{std::move(selected_backtest_handle)}
  , backtest_handles_{std::move(backtest_handles)}
  , asset_handles_{std::move(asset_handles)}
  , strategy_handles_{std::move(strategy_handles)}
  , market_handles_{std::move(market_handles)}
  , broker_handles_{std::move(broker_handles)}
  , profile_handles_{std::move(profile_handles)}
  {
  }

  auto selected_backtest_handle(this const StateDescriptor& self) noexcept
   -> backtest::StoreHandle<backtest::Backtest>
  {
    return self.selected_backtest_handle_;
  }

  void selected_backtest_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
  {
    self.selected_backtest_handle_ = handle;
  }

  auto backtest_handles(this const StateDescriptor& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Backtest>>&
  {
    return self.backtest_handles_;
  }

  auto backtest_handles(this StateDescriptor& self) noexcept
   -> std::vector<backtest::StoreHandle<backtest::Backtest>>&
  {
    return self.backtest_handles_;
  }

  void add_backtest_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Backtest> backtest_handle) noexcept
  {
    self.backtest_handles_.push_back(backtest_handle);
  }

  void remove_backtest_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Backtest> backtest_handle) noexcept
  {
    auto& backtest_handles = self.backtest_handles_;
    const auto it = std::ranges::find(backtest_handles, backtest_handle);
    if(it != backtest_handles.end()) {
      backtest_handles.erase(it);
    }
  }

  auto asset_handles(this const StateDescriptor& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Asset>>&
  {
    return self.asset_handles_;
  }

  void
  add_asset_handle(this StateDescriptor& self,
                   backtest::StoreHandle<backtest::Asset> asset_handle) noexcept
  {
    self.asset_handles_.push_back(asset_handle);
  }

  void remove_asset_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Asset> asset_handle) noexcept
  {
    auto& asset_handles = self.asset_handles_;
    const auto it = std::ranges::find(asset_handles, asset_handle);
    if(it != asset_handles.end()) {
      asset_handles.erase(it);
    }
  }

  auto strategy_handles(this const StateDescriptor& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Strategy>>&
  {
    return self.strategy_handles_;
  }

  void add_strategy_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Strategy> strategy_handle) noexcept
  {
    self.strategy_handles_.push_back(strategy_handle);
  }

  void remove_strategy_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Strategy> strategy_handle) noexcept
  {
    auto& strategy_handles = self.strategy_handles_;
    const auto it = std::ranges::find(strategy_handles, strategy_handle);
    if(it != strategy_handles.end()) {
      strategy_handles.erase(it);
    }
  }

  auto market_handles(this const StateDescriptor& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Market>>&
  {
    return self.market_handles_;
  }

  void add_market_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Market> market_handle) noexcept
  {
    self.market_handles_.push_back(market_handle);
  }

  void remove_market_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Market> market_handle) noexcept
  {
    auto& market_handles = self.market_handles_;
    const auto it = std::ranges::find(market_handles, market_handle);
    if(it != market_handles.end()) {
      market_handles.erase(it);
    }
  }

  auto broker_handles(this const StateDescriptor& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Broker>>&
  {
    return self.broker_handles_;
  }

  void add_broker_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Broker> broker_handle) noexcept
  {
    self.broker_handles_.push_back(broker_handle);
  }

  void remove_broker_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Broker> broker_handle) noexcept
  {
    auto& broker_handles = self.broker_handles_;
    const auto it = std::ranges::find(broker_handles, broker_handle);
    if(it != broker_handles.end()) {
      broker_handles.erase(it);
    }
  }

  auto profile_handles(this const StateDescriptor& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Profile>>&
  {
    return self.profile_handles_;
  }

  void add_profile_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Profile> profile_handle) noexcept
  {
    self.profile_handles_.push_back(profile_handle);
  }

  void remove_profile_handle(
   this StateDescriptor& self,
   backtest::StoreHandle<backtest::Profile> profile_handle) noexcept
  {
    auto& profile_handles = self.profile_handles_;
    const auto it = std::ranges::find(profile_handles, profile_handle);
    if(it != profile_handles.end()) {
      profile_handles.erase(it);
    }
  }

private:
  backtest::StoreHandle<backtest::Backtest> selected_backtest_handle_;

  std::vector<backtest::StoreHandle<backtest::Backtest>> backtest_handles_;
  std::vector<backtest::StoreHandle<backtest::Asset>> asset_handles_;
  std::vector<backtest::StoreHandle<backtest::Strategy>> strategy_handles_;
  std::vector<backtest::StoreHandle<backtest::Market>> market_handles_;
  std::vector<backtest::StoreHandle<backtest::Broker>> broker_handles_;
  std::vector<backtest::StoreHandle<backtest::Profile>> profile_handles_;
};

} // namespace pludux::apps
