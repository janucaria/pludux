module;

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <format>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

export module pludux.backtest:backtest;

import pludux;

import :store;
import :asset;
import :profile;
import :trade_entry;
import :trade_exit;
import :trade_record;
import :trade_position;
import :trade_session;
import :backtest_summary;
import :strategy;
import :broker;
import :market;

export namespace pludux::backtest {

class Backtest {
public:
  Backtest()
  : Backtest{"",
             1'000'000,
             StoreHandle<Asset>{},
             StoreHandle<Strategy>{},
             StoreHandle<Market>{},
             StoreHandle<Broker>{},
             StoreHandle<Profile>{}}
  {
  }

  Backtest(std::string name,
           double initial_capital,
           StoreHandle<Asset> asset_handle,
           StoreHandle<Strategy> strategy_handle,
           StoreHandle<Market> market_handle,
           StoreHandle<Broker> broker_handle,
           StoreHandle<Profile> profile_handle)
  : Backtest{std::move(name),
             initial_capital,
             std::move(asset_handle),
             std::move(strategy_handle),
             std::move(market_handle),
             std::move(broker_handle),
             std::move(profile_handle),
             false}
  {
  }

  Backtest(std::string name,
           double initial_capital,
           StoreHandle<Asset> asset_handle,
           StoreHandle<Strategy> strategy_handle,
           StoreHandle<Market> market_handle,
           StoreHandle<Broker> broker_handle,
           StoreHandle<Profile> profile_handle,
           bool is_failed)
  : name_{std::move(name)}
  , initial_capital_{initial_capital}
  , asset_handle_{std::move(asset_handle)}
  , strategy_handle_{std::move(strategy_handle)}
  , market_handle_{std::move(market_handle)}
  , broker_handle_{std::move(broker_handle)}
  , profile_handle_{std::move(profile_handle)}
  , is_failed_{is_failed}
  {
  }

  auto name(this const Backtest& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Backtest& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

  auto initial_capital(this const Backtest& self) noexcept -> double
  {
    return self.initial_capital_;
  }

  void initial_capital(this Backtest& self, double initial_capital) noexcept
  {
    self.initial_capital_ = initial_capital;
  }

  auto asset_handle(this const Backtest& self) noexcept -> StoreHandle<Asset>
  {
    return self.asset_handle_;
  }

  void asset_handle(this Backtest& self,
                    StoreHandle<Asset> new_asset_handle) noexcept
  {
    self.asset_handle_ = std::move(new_asset_handle);
  }

  auto strategy_handle(this const Backtest& self) noexcept
   -> StoreHandle<Strategy>
  {
    return self.strategy_handle_;
  }

  void strategy_handle(this Backtest& self,
                       StoreHandle<Strategy> new_strategy_handle) noexcept
  {
    self.strategy_handle_ = std::move(new_strategy_handle);
  }

  auto market_handle(this const Backtest& self) noexcept -> StoreHandle<Market>
  {
    return self.market_handle_;
  }

  void market_handle(this Backtest& self,
                     StoreHandle<Market> new_market_handle) noexcept
  {
    self.market_handle_ = std::move(new_market_handle);
  }

  auto broker_handle(this const Backtest& self) noexcept -> StoreHandle<Broker>
  {
    return self.broker_handle_;
  }

  void broker_handle(this Backtest& self,
                     StoreHandle<Broker> new_broker_handle) noexcept
  {
    self.broker_handle_ = std::move(new_broker_handle);
  }

  auto profile_handle(this const Backtest& self) noexcept
   -> StoreHandle<Profile>
  {
    return self.profile_handle_;
  }

  void profile_handle(this Backtest& self,
                      StoreHandle<Profile> new_profile_handle) noexcept
  {
    self.profile_handle_ = std::move(new_profile_handle);
  }

  auto is_failed(this const Backtest& self) noexcept -> bool
  {
    return self.is_failed_;
  }

  void is_failed(this Backtest& self, bool is_failed) noexcept
  {
    self.is_failed_ = is_failed;
  }

  auto equivalent_rules(this const Backtest& self,
                        const Backtest& other) noexcept -> bool
  {
    return self.initial_capital_ == other.initial_capital_ &&
           self.asset_handle() == other.asset_handle() &&
           self.strategy_handle() == other.strategy_handle() &&
           self.market_handle() == other.market_handle() &&
           self.broker_handle() == other.broker_handle() &&
           self.profile_handle() == other.profile_handle();
  }

  auto equivalent_rules_and_metadata(this const Backtest& self,
                                     const Backtest& other) noexcept -> bool
  {
    return self.name_ == other.name_ && self.equivalent_rules(other);
  }

private:
  std::string name_;
  double initial_capital_;

  StoreHandle<Asset> asset_handle_;
  StoreHandle<Strategy> strategy_handle_;
  StoreHandle<Market> market_handle_;
  StoreHandle<Broker> broker_handle_;
  StoreHandle<Profile> profile_handle_;

  bool is_failed_;
};

} // namespace pludux::backtest
