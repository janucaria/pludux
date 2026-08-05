module;

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:backtest;

import pludux;

import :store_handle;
import :strategy_performance;

export namespace pludux::backtest {

class Backtest {
public:
  Backtest()
  : Backtest{"",
             AssetStoreHandle{},
             StrategyStoreHandle{},
             ProfileStoreHandle{},
             {},
             StrategyPerformanceConfig{}}
  {
  }

  Backtest(std::string name,
           AssetStoreHandle asset_handle,
           StrategyStoreHandle strategy_handle,
           ProfileStoreHandle profile_handle,
           std::vector<NumericInputNode> inputs = {},
           StrategyPerformanceConfig strategy_performance = {})
  : name_{std::move(name)}
  , asset_handle_{std::move(asset_handle)}
  , strategy_handle_{std::move(strategy_handle)}
  , profile_handle_{std::move(profile_handle)}
  , inputs_{std::move(inputs)}
  , strategy_performance_{std::move(strategy_performance)}
  {
  }

  auto operator==(const Backtest&) const noexcept -> bool = default;

  auto name(this const Backtest& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Backtest& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

  auto asset_handle(this const Backtest& self) noexcept -> AssetStoreHandle
  {
    return self.asset_handle_;
  }

  void asset_handle(this Backtest& self,
                    AssetStoreHandle new_asset_handle) noexcept
  {
    self.asset_handle_ = std::move(new_asset_handle);
  }

  auto strategy_handle(this const Backtest& self) noexcept
   -> StrategyStoreHandle
  {
    return self.strategy_handle_;
  }

  void strategy_handle(this Backtest& self,
                       StrategyStoreHandle new_strategy_handle) noexcept
  {
    self.strategy_handle_ = std::move(new_strategy_handle);
  }

  auto profile_handle(this const Backtest& self) noexcept -> ProfileStoreHandle
  {
    return self.profile_handle_;
  }

  void profile_handle(this Backtest& self,
                      ProfileStoreHandle new_profile_handle) noexcept
  {
    self.profile_handle_ = std::move(new_profile_handle);
  }

  auto inputs(this const Backtest& self) noexcept
   -> const std::vector<NumericInputNode>&
  {
    return self.inputs_;
  }

  void inputs(this Backtest& self,
              std::vector<NumericInputNode> new_inputs) noexcept
  {
    self.inputs_ = std::move(new_inputs);
  }

  auto strategy_performance(this const Backtest& self) noexcept
   -> const StrategyPerformanceConfig&
  {
    return self.strategy_performance_;
  }

  void strategy_performance(this Backtest& self,
                            StrategyPerformanceConfig strategy_performance)
  {
    strategy_performance.validate();
    self.strategy_performance_ = std::move(strategy_performance);
  }

  auto equivalent_rules(this const Backtest& self,
                        const Backtest& other) noexcept -> bool
  {
    return self.asset_handle() == other.asset_handle() &&
           self.strategy_handle() == other.strategy_handle() &&
           self.profile_handle() == other.profile_handle() &&
           self.inputs_ == other.inputs_ &&
           self.strategy_performance_ == other.strategy_performance_;
  }

private:
  std::string name_;

  AssetStoreHandle asset_handle_;
  StrategyStoreHandle strategy_handle_;
  ProfileStoreHandle profile_handle_;

  std::vector<NumericInputNode> inputs_;
  StrategyPerformanceConfig strategy_performance_;
};

} // namespace pludux::backtest
