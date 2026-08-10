module;

#include <utility>
#include <vector>

export module pludux.backtest:backtest_setup;

import pludux;

import :entry_filter_method_context;
import :store_handle;

export namespace pludux::backtest {

enum class FailsafeActivation { Always, PreviousSetupEntryFilteredPosition };

class BacktestSetup {
public:
  BacktestSetup() = default;

  BacktestSetup(StrategyStoreHandle strategy_handle,
                ProfileStoreHandle profile_handle,
                std::vector<NumericInputNode> inputs = {},
                ErasedNode<EntryFilterMethodContext> entry_filter = TrueNode{})
  : strategy_handle_{std::move(strategy_handle)}
  , profile_handle_{std::move(profile_handle)}
  , inputs_{std::move(inputs)}
  , entry_filter_{std::move(entry_filter)}
  {
  }

  auto operator==(const BacktestSetup&) const noexcept -> bool = default;

  auto strategy_handle(this const BacktestSetup& self) noexcept
   -> StrategyStoreHandle
  {
    return self.strategy_handle_;
  }

  void strategy_handle(this BacktestSetup& self,
                       StrategyStoreHandle value) noexcept
  {
    self.strategy_handle_ = std::move(value);
  }

  auto profile_handle(this const BacktestSetup& self) noexcept
   -> ProfileStoreHandle
  {
    return self.profile_handle_;
  }

  void profile_handle(this BacktestSetup& self,
                      ProfileStoreHandle value) noexcept
  {
    self.profile_handle_ = std::move(value);
  }

  auto inputs(this const BacktestSetup& self) noexcept
   -> const std::vector<NumericInputNode>&
  {
    return self.inputs_;
  }

  void inputs(this BacktestSetup& self,
              std::vector<NumericInputNode> value) noexcept
  {
    self.inputs_ = std::move(value);
  }

  auto entry_filter(this const BacktestSetup& self) noexcept
   -> const ErasedNode<EntryFilterMethodContext>&
  {
    return self.entry_filter_;
  }

  void entry_filter(this BacktestSetup& self,
                    ErasedNode<EntryFilterMethodContext> value) noexcept
  {
    self.entry_filter_ = std::move(value);
  }

  auto equivalent_rules(this const BacktestSetup& self,
                        const BacktestSetup& other) noexcept -> bool
  {
    return self == other;
  }

private:
  StrategyStoreHandle strategy_handle_;
  ProfileStoreHandle profile_handle_;
  std::vector<NumericInputNode> inputs_;
  ErasedNode<EntryFilterMethodContext> entry_filter_{TrueNode{}};
};

class BacktestFailsafeSetup {
public:
  BacktestFailsafeSetup() = default;

  BacktestFailsafeSetup(
   BacktestSetup setup,
   FailsafeActivation activation = FailsafeActivation::Always) noexcept
  : setup_{std::move(setup)}
  , activation_{activation}
  {
  }

  auto operator==(const BacktestFailsafeSetup&) const noexcept
   -> bool = default;

  auto setup(this const BacktestFailsafeSetup& self) noexcept
   -> const BacktestSetup&
  {
    return self.setup_;
  }

  auto setup(this BacktestFailsafeSetup& self) noexcept -> BacktestSetup&
  {
    return self.setup_;
  }

  void setup(this BacktestFailsafeSetup& self, BacktestSetup value) noexcept
  {
    self.setup_ = std::move(value);
  }

  auto activation(this const BacktestFailsafeSetup& self) noexcept
   -> FailsafeActivation
  {
    return self.activation_;
  }

  void activation(this BacktestFailsafeSetup& self,
                  FailsafeActivation value) noexcept
  {
    self.activation_ = value;
  }

  auto equivalent_rules(this const BacktestFailsafeSetup& self,
                        const BacktestFailsafeSetup& other) noexcept -> bool
  {
    return self == other;
  }

private:
  BacktestSetup setup_;
  FailsafeActivation activation_{FailsafeActivation::Always};
};

} // namespace pludux::backtest
