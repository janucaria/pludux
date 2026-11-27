module;

#include <map>
#include <string>

#include <utility>
#include <vector>

export module pludux.backtest:strategy_setup;

import pludux;

import :entry_filter_method_context;
import :store_handle;
import :calls;

export namespace pludux::backtest {

enum class AlternateStrategyActivation {
  Always,
  // The preceding setup's entry-filtered theoretical position is still open.
  Failsafe
};

class StrategySetup {
public:
  explicit StrategySetup(
   StrategyStoreHandle strategy_handle = {},
   std::vector<NumericInputNode> inputs = {},
   ErasedNode<EntryFilterMethodContext> entry_filter = TrueNode{},
   CallMappings call_mappings = {})
  : strategy_handle_{strategy_handle}
  , inputs_{std::move(inputs)}
  , entry_filter_{std::move(entry_filter)}
  , call_mappings_{std::move(call_mappings)}
  {
  }

  auto operator==(const StrategySetup& other) const noexcept -> bool
  {
    return strategy_handle_ == other.strategy_handle_ &&
           inputs_ == other.inputs_ && entry_filter_ == other.entry_filter_ &&
           call_mappings_ == other.call_mappings_;
  }

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

  auto call_mappings(this auto& self) noexcept -> auto&
  {
    return self.call_mappings_;
  }

private:
  StrategyStoreHandle strategy_handle_;
  std::vector<NumericInputNode> inputs_;
  ErasedNode<EntryFilterMethodContext> entry_filter_{TrueNode{}};
  CallMappings call_mappings_;
};

class SystemAlternateStrategy {
public:
  SystemAlternateStrategy() = default;

  SystemAlternateStrategy(StrategySetup setup,
                          AlternateStrategyActivation activation =
                           AlternateStrategyActivation::Always) noexcept
  : setup_{std::move(setup)}
  , activation_{activation}
  {
  }

  auto operator==(const SystemAlternateStrategy&) const noexcept
   -> bool = default;

  auto setup(this auto& self) noexcept -> auto&
  {
    return self.setup_;
  }

  auto activation(this const SystemAlternateStrategy& self) noexcept
   -> AlternateStrategyActivation
  {
    return self.activation_;
  }

  void activation(this SystemAlternateStrategy& self,
                  AlternateStrategyActivation value) noexcept
  {
    self.activation_ = value;
  }

private:
  StrategySetup setup_;
  AlternateStrategyActivation activation_{
   AlternateStrategyActivation::Always};
};

} // namespace pludux::backtest
