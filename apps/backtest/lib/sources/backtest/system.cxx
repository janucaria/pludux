module;

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:system;

import :strategy;
import :store_handle;
import :model_performance;

export namespace pludux::backtest {

class System {
public:
  System() = default;

  System(std::string name,
         WatchlistStoreHandle watchlist_handle,
         ModelPerformanceConfig model_performance,
         StrategyStoreHandle main_strategy_handle,
         std::vector<SystemFailsafeStrategy> failsafe_strategies = {})
  : name_{std::move(name)}
  , watchlist_handle_{std::move(watchlist_handle)}
  , model_performance_{std::move(model_performance)}
  , main_strategy_handle_{std::move(main_strategy_handle)}
  , failsafe_strategies_{std::move(failsafe_strategies)}
  {
    model_performance_.validate();
  }

  auto operator==(const System&) const noexcept -> bool = default;

  auto name(this const System& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this System& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

  auto watchlist_handle(this const System& self) noexcept
   -> WatchlistStoreHandle
  {
    return self.watchlist_handle_;
  }

  void watchlist_handle(this System& self,
                        WatchlistStoreHandle new_watchlist_handle) noexcept
  {
    self.watchlist_handle_ = std::move(new_watchlist_handle);
  }

  auto model_performance(this const System& self) noexcept
   -> const ModelPerformanceConfig&
  {
    return self.model_performance_;
  }

  void model_performance(this System& self,
                             ModelPerformanceConfig value)
  {
    value.validate();
    self.model_performance_ = std::move(value);
  }

  auto main_strategy_handle(this const System& self) noexcept
   -> StrategyStoreHandle
  {
    return self.main_strategy_handle_;
  }

  void main_strategy_handle(this System& self, StrategyStoreHandle value) noexcept
  {
    self.main_strategy_handle_ = std::move(value);
  }

  auto failsafe_strategies(this const System& self) noexcept
    -> const std::vector<SystemFailsafeStrategy>&
  {
    return self.failsafe_strategies_;
  }

  auto failsafe_strategies(this System& self) noexcept
    -> std::vector<SystemFailsafeStrategy>&
  {
    return self.failsafe_strategies_;
  }

  void failsafe_strategies(this System& self,
                           std::vector<SystemFailsafeStrategy> value) noexcept
  {
    self.failsafe_strategies_ = std::move(value);
  }

  auto strategy_count(this const System& self) noexcept -> std::size_t
  {
    return 1 + self.failsafe_strategies_.size();
  }

  auto strategy_handle(this const System& self, std::size_t index)
    -> StrategyStoreHandle
  {
    return index == 0 ? self.main_strategy_handle_
                      : self.failsafe_strategies_.at(index - 1).strategy_handle();
  }

  auto equivalent_rules(this const System& self,
                        const System& other) noexcept -> bool
  {
    return self.watchlist_handle() == other.watchlist_handle() &&
           self.model_performance_ == other.model_performance_ &&
            self.main_strategy_handle_ == other.main_strategy_handle_ &&
            self.failsafe_strategies_ == other.failsafe_strategies_;
  }

private:
  std::string name_;

  WatchlistStoreHandle watchlist_handle_;
  ModelPerformanceConfig model_performance_;
  StrategyStoreHandle main_strategy_handle_;
  std::vector<SystemFailsafeStrategy> failsafe_strategies_;
};

} // namespace pludux::backtest
