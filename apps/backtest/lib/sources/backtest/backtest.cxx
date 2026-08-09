module;

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:backtest;

import :backtest_setup;
import :store_handle;
import :strategy_performance;

export namespace pludux::backtest {

class Backtest {
public:
  Backtest() = default;

  Backtest(std::string name,
           WatchlistStoreHandle watchlist_handle,
           StrategyPerformanceConfig strategy_performance,
           BacktestSetup main_setup,
           std::vector<BacktestFailsafeSetup> failsafe_setups = {})
  : name_{std::move(name)}
  , watchlist_handle_{std::move(watchlist_handle)}
  , strategy_performance_{std::move(strategy_performance)}
  , main_setup_{std::move(main_setup)}
  , failsafe_setups_{std::move(failsafe_setups)}
  {
    strategy_performance_.validate();
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

  auto watchlist_handle(this const Backtest& self) noexcept
   -> WatchlistStoreHandle
  {
    return self.watchlist_handle_;
  }

  void watchlist_handle(this Backtest& self,
                        WatchlistStoreHandle new_watchlist_handle) noexcept
  {
    self.watchlist_handle_ = std::move(new_watchlist_handle);
  }

  auto strategy_performance(this const Backtest& self) noexcept
   -> const StrategyPerformanceConfig&
  {
    return self.strategy_performance_;
  }

  void strategy_performance(this Backtest& self,
                            StrategyPerformanceConfig value)
  {
    value.validate();
    self.strategy_performance_ = std::move(value);
  }

  auto main_setup(this const Backtest& self) noexcept -> const BacktestSetup&
  {
    return self.main_setup_;
  }

  auto main_setup(this Backtest& self) noexcept -> BacktestSetup&
  {
    return self.main_setup_;
  }

  void main_setup(this Backtest& self, BacktestSetup value) noexcept
  {
    self.main_setup_ = std::move(value);
  }

  auto failsafe_setups(this const Backtest& self) noexcept
   -> const std::vector<BacktestFailsafeSetup>&
  {
    return self.failsafe_setups_;
  }

  auto failsafe_setups(this Backtest& self) noexcept
   -> std::vector<BacktestFailsafeSetup>&
  {
    return self.failsafe_setups_;
  }

  void failsafe_setups(this Backtest& self,
                       std::vector<BacktestFailsafeSetup> value) noexcept
  {
    self.failsafe_setups_ = std::move(value);
  }

  auto setup_count(this const Backtest& self) noexcept -> std::size_t
  {
    return 1 + self.failsafe_setups_.size();
  }

  auto setup(this const Backtest& self, std::size_t index)
   -> const BacktestSetup&
  {
    return index == 0 ? self.main_setup_
                      : self.failsafe_setups_.at(index - 1).setup();
  }

  auto setup(this Backtest& self, std::size_t index) -> BacktestSetup&
  {
    return index == 0 ? self.main_setup_
                      : self.failsafe_setups_.at(index - 1).setup();
  }

  auto references_strategy(this const Backtest& self,
                           StrategyStoreHandle handle) noexcept -> bool
  {
    for(auto index = std::size_t{}; index < self.setup_count(); ++index) {
      if(self.setup(index).strategy_handle() == handle) {
        return true;
      }
    }
    return false;
  }

  auto references_profile(this const Backtest& self,
                          ProfileStoreHandle handle) noexcept -> bool
  {
    for(auto index = std::size_t{}; index < self.setup_count(); ++index) {
      if(self.setup(index).profile_handle() == handle) {
        return true;
      }
    }
    return false;
  }

  auto equivalent_rules(this const Backtest& self,
                        const Backtest& other) noexcept -> bool
  {
    return self.watchlist_handle() == other.watchlist_handle() &&
           self.strategy_performance_ == other.strategy_performance_ &&
           self.main_setup_.equivalent_rules(other.main_setup_) &&
           self.failsafe_setups_ == other.failsafe_setups_;
  }

private:
  std::string name_;

  WatchlistStoreHandle watchlist_handle_;
  StrategyPerformanceConfig strategy_performance_;
  BacktestSetup main_setup_;
  std::vector<BacktestFailsafeSetup> failsafe_setups_;
};

} // namespace pludux::backtest
