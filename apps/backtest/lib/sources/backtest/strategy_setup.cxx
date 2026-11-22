module;

#include <utility>
#include <vector>

export module pludux.backtest:strategy_setup;

import pludux;

import :entry_filter_method_context;
import :store_handle;

export namespace pludux::backtest {

enum class FailsafeStrategyActivation {
  Always,
  PreviousStrategyEntryFilteredPosition
};

class StrategySetup {
public:
  explicit StrategySetup(
   StrategyStoreHandle strategy_handle = {},
   std::vector<NumericInputNode> inputs = {},
   ErasedNode<EntryFilterMethodContext> entry_filter = TrueNode{})
  : strategy_handle_{strategy_handle}
  , inputs_{std::move(inputs)}
  , entry_filter_{std::move(entry_filter)}
  {
  }

  auto operator==(const StrategySetup&) const noexcept -> bool = default;

  auto strategy_handle(this const StrategySetup& self) noexcept
   -> StrategyStoreHandle
  {
    return self.strategy_handle_;
  }

  void strategy_handle(this StrategySetup& self,
                       StrategyStoreHandle value) noexcept
  {
    self.strategy_handle_ = std::move(value);
  }

  auto inputs(this const StrategySetup& self) noexcept
   -> const std::vector<NumericInputNode>&
  {
    return self.inputs_;
  }

  void inputs(this StrategySetup& self,
              std::vector<NumericInputNode> value) noexcept
  {
    self.inputs_ = std::move(value);
  }

  auto entry_filter(this const StrategySetup& self) noexcept
   -> const ErasedNode<EntryFilterMethodContext>&
  {
    return self.entry_filter_;
  }

  void entry_filter(this StrategySetup& self,
                    ErasedNode<EntryFilterMethodContext> value) noexcept
  {
    self.entry_filter_ = std::move(value);
  }

private:
  StrategyStoreHandle strategy_handle_;
  std::vector<NumericInputNode> inputs_;
  ErasedNode<EntryFilterMethodContext> entry_filter_{TrueNode{}};
};

class SystemFailsafeStrategy {
public:
  SystemFailsafeStrategy() = default;

  SystemFailsafeStrategy(StrategySetup setup,
                         FailsafeStrategyActivation activation =
                          FailsafeStrategyActivation::Always) noexcept
  : setup_{std::move(setup)}
  , activation_{activation}
  {
  }

  auto operator==(const SystemFailsafeStrategy&) const noexcept
   -> bool = default;

  auto setup(this auto& self) noexcept -> auto&
  {
    return self.setup_;
  }

  auto activation(this const SystemFailsafeStrategy& self) noexcept
   -> FailsafeStrategyActivation
  {
    return self.activation_;
  }

  void activation(this SystemFailsafeStrategy& self,
                  FailsafeStrategyActivation value) noexcept
  {
    self.activation_ = value;
  }

private:
  StrategySetup setup_;
  FailsafeStrategyActivation activation_{FailsafeStrategyActivation::Always};
};

} // namespace pludux::backtest
