module;

#include <map>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:system;

import :strategy_setup;
import :store_handle;
import :calls;
import :strategy_performance;

export namespace pludux::backtest {

class System {
public:
  System() = default;

  System(std::string name,
         WatchlistStoreHandle watchlist_handle,
         ProfileStoreHandle profile_handle,
         StrategyPerformanceConfig strategy_performance,
         StrategySetup main_setup,
         std::vector<SystemFailsafeStrategy> failsafe_strategies = {},
         CallStoreHandle call_handle = {},
         CallColumns call_columns = {},
         EntryPriority entry_priority = EntryPriority::CallFirst)
  : name_{std::move(name)}
  , watchlist_handle_{std::move(watchlist_handle)}
  , profile_handle_{profile_handle}
  , strategy_performance_{std::move(strategy_performance)}
  , main_setup_{std::move(main_setup)}
  , failsafe_strategies_{std::move(failsafe_strategies)}
  , call_handle_{call_handle}
  , call_columns_{std::move(call_columns)}
  , entry_priority_{entry_priority}
  {
    strategy_performance_.validate();
  }

  auto operator==(const System& other) const noexcept -> bool
  {
    return name_ == other.name_ && equivalent_rules(other);
  }

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

  auto strategy_performance(this const System& self) noexcept
   -> const StrategyPerformanceConfig&
  {
    return self.strategy_performance_;
  }

  void strategy_performance(this System& self, StrategyPerformanceConfig value)
  {
    value.validate();
    self.strategy_performance_ = std::move(value);
  }

  auto main_setup(this auto& self) noexcept -> auto&
  {
    return self.main_setup_;
  }

  auto profile_handle(this const System& self) noexcept -> ProfileStoreHandle
  {
    return self.profile_handle_;
  }

  void profile_handle(this System& self, ProfileStoreHandle handle) noexcept
  {
    self.profile_handle_ = handle;
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

  auto strategy_setup(this auto& self, std::size_t index) -> auto&
  {
    return index == 0 ? self.main_setup_
                      : self.failsafe_strategies_.at(index - 1).setup();
  }

  auto strategy_handle(this const System& self, std::size_t index)
   -> StrategyStoreHandle
  {
    return self.strategy_setup(index).strategy_handle();
  }

  auto call_handle(this const System& self) noexcept -> CallStoreHandle
  {
    return self.call_handle_;
  }
  void call_handle(this System& self, CallStoreHandle handle) noexcept
  {
    self.call_handle_ = handle;
  }
  auto call_columns(this auto& self) noexcept -> auto&
  {
    return self.call_columns_;
  }
  void call_columns(this System& self, CallColumns columns) noexcept
  {
    self.call_columns_ = std::move(columns);
  }

  auto entry_priority() const noexcept -> EntryPriority
  {
    return entry_priority_;
  }
  void entry_priority(EntryPriority value) noexcept
  {
    entry_priority_ = value;
  }

  auto equivalent_rules(this const System& self, const System& other) noexcept
   -> bool
  {
    return self.watchlist_handle() == other.watchlist_handle() &&
           self.profile_handle_ == other.profile_handle_ &&
           self.strategy_performance_ == other.strategy_performance_ &&
           self.call_handle_ == other.call_handle_ &&
           self.call_columns_ == other.call_columns_ &&
           self.entry_priority_ == other.entry_priority_ &&
           self.main_setup_ == other.main_setup_ &&
           self.failsafe_strategies_ == other.failsafe_strategies_;
  }

private:
  std::string name_;

  WatchlistStoreHandle watchlist_handle_;
  ProfileStoreHandle profile_handle_;
  StrategyPerformanceConfig strategy_performance_;
  StrategySetup main_setup_;
  std::vector<SystemFailsafeStrategy> failsafe_strategies_;
  CallStoreHandle call_handle_;
  CallColumns call_columns_;
  EntryPriority entry_priority_{EntryPriority::CallFirst};
};

} // namespace pludux::backtest
