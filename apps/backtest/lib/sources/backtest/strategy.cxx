module;

#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:strategy;

import pludux;

import :entry_filter_method_context;
import :store_handle;

export namespace pludux::backtest {

enum class FailsafeStrategyActivation {
  Always,
  PreviousStrategyEntryFilteredPosition
};

class Strategy {
public:
  Strategy()
  : Strategy{""}
  {
  }

  Strategy(std::string name,
            ModelStoreHandle model_handle = {},
            ProfileStoreHandle profile_handle = {},
            std::vector<NumericInputNode> inputs = {},
            ErasedNode<EntryFilterMethodContext> entry_filter = TrueNode{})
  : name_{std::move(name)}
  , model_handle_{std::move(model_handle)}
  , profile_handle_{std::move(profile_handle)}
  , inputs_{std::move(inputs)}
  , entry_filter_{std::move(entry_filter)}
  {
  }

  auto operator==(const Strategy&) const noexcept -> bool = default;

  auto name(this const Strategy& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Strategy& self, std::string value) noexcept
  {
    self.name_ = std::move(value);
  }

  auto model_handle(this const Strategy& self) noexcept -> ModelStoreHandle
  {
    return self.model_handle_;
  }

  void model_handle(this Strategy& self, ModelStoreHandle value) noexcept
  {
    self.model_handle_ = std::move(value);
  }

  auto profile_handle(this const Strategy& self) noexcept -> ProfileStoreHandle
  {
    return self.profile_handle_;
  }

  void profile_handle(this Strategy& self, ProfileStoreHandle value) noexcept
  {
    self.profile_handle_ = std::move(value);
  }

  auto inputs(this const Strategy& self) noexcept
   -> const std::vector<NumericInputNode>&
  {
    return self.inputs_;
  }

  void inputs(this Strategy& self, std::vector<NumericInputNode> value) noexcept
  {
    self.inputs_ = std::move(value);
  }

  auto entry_filter(this const Strategy& self) noexcept
   -> const ErasedNode<EntryFilterMethodContext>&
  {
    return self.entry_filter_;
  }

  void entry_filter(this Strategy& self,
                    ErasedNode<EntryFilterMethodContext> value) noexcept
  {
    self.entry_filter_ = std::move(value);
  }

  auto equivalent_rules(this const Strategy& self, const Strategy& other) noexcept
   -> bool
  {
    return self.model_handle_ == other.model_handle_ &&
           self.profile_handle_ == other.profile_handle_ &&
           self.inputs_ == other.inputs_ && self.entry_filter_ == other.entry_filter_;
  }

  auto references_model(this const Strategy& self,
                        ModelStoreHandle handle) noexcept -> bool
  {
    return self.model_handle_ == handle;
  }

  auto references_profile(this const Strategy& self,
                          ProfileStoreHandle handle) noexcept -> bool
  {
    return self.profile_handle_ == handle;
  }

private:
  std::string name_;
  ModelStoreHandle model_handle_;
  ProfileStoreHandle profile_handle_;
  std::vector<NumericInputNode> inputs_;
  ErasedNode<EntryFilterMethodContext> entry_filter_{TrueNode{}};
};

class SystemFailsafeStrategy {
public:
  SystemFailsafeStrategy() = default;

  SystemFailsafeStrategy(
   StrategyStoreHandle strategy_handle,
   FailsafeStrategyActivation activation = FailsafeStrategyActivation::Always) noexcept
  : strategy_handle_{std::move(strategy_handle)}
  , activation_{activation}
  {
  }

  auto operator==(const SystemFailsafeStrategy&) const noexcept -> bool = default;

  auto strategy_handle(this const SystemFailsafeStrategy& self) noexcept
    -> StrategyStoreHandle
  {
    return self.strategy_handle_;
  }

  void strategy_handle(this SystemFailsafeStrategy& self,
                       StrategyStoreHandle value) noexcept
  {
    self.strategy_handle_ = std::move(value);
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

  auto equivalent_rules(this const SystemFailsafeStrategy& self,
                        const SystemFailsafeStrategy& other) noexcept -> bool
  {
    return self == other;
  }

private:
  StrategyStoreHandle strategy_handle_;
  FailsafeStrategyActivation activation_{FailsafeStrategyActivation::Always};
};

} // namespace pludux::backtest
