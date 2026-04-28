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

export module pludux.apps.backtest:ui_state;

import pludux.backtest;

export namespace pludux::apps {

class UiState {
public:
  UiState() = default;

  UiState(std::string imgui_ini_settings,
          backtest::BacktestStoreHandle selected_backtest_handle,
          std::vector<backtest::BacktestStoreHandle> backtest_handles,
          std::vector<backtest::AssetStoreHandle> asset_handles,
          std::vector<backtest::StrategyStoreHandle> strategy_handles,
          std::vector<backtest::MarketStoreHandle> market_handles,
          std::vector<backtest::BrokerStoreHandle> broker_handles,
          std::vector<backtest::ProfileStoreHandle> profile_handles)
  : imgui_ini_settings_{std::move(imgui_ini_settings)}
  , selected_backtest_handle_{std::move(selected_backtest_handle)}
  , backtest_handles_{std::move(backtest_handles)}
  , asset_handles_{std::move(asset_handles)}
  , strategy_handles_{std::move(strategy_handles)}
  , market_handles_{std::move(market_handles)}
  , broker_handles_{std::move(broker_handles)}
  , profile_handles_{std::move(profile_handles)}
  {
  }

  auto imgui_ini_settings(this const UiState& self) noexcept
   -> const std::string&
  {
    return self.imgui_ini_settings_;
  }

  void imgui_ini_settings(this UiState& self,
                          std::string imgui_ini_settings) noexcept
  {
    self.imgui_ini_settings_ = std::move(imgui_ini_settings);
  }

  auto selected_backtest_handle(this const UiState& self) noexcept
   -> backtest::BacktestStoreHandle
  {
    return self.selected_backtest_handle_;
  }

  void selected_backtest_handle(this UiState& self,
                                backtest::BacktestStoreHandle handle) noexcept
  {
    self.selected_backtest_handle_ = handle;
  }

  auto backtest_handles(this const UiState& self) noexcept
   -> const std::vector<backtest::BacktestStoreHandle>&
  {
    return self.backtest_handles_;
  }

  auto backtest_handles(this UiState& self) noexcept
   -> std::vector<backtest::BacktestStoreHandle>&
  {
    return self.backtest_handles_;
  }

  void
  add_backtest_handle(this UiState& self,
                      backtest::BacktestStoreHandle backtest_handle) noexcept
  {
    self.backtest_handles_.push_back(backtest_handle);
  }

  void
  remove_backtest_handle(this UiState& self,
                         backtest::BacktestStoreHandle backtest_handle) noexcept
  {
    auto& backtest_handles = self.backtest_handles_;
    const auto it = std::ranges::find(backtest_handles, backtest_handle);
    if(it != backtest_handles.end()) {
      backtest_handles.erase(it);
    }
  }

  auto asset_handles(this const UiState& self) noexcept
   -> const std::vector<backtest::AssetStoreHandle>&
  {
    return self.asset_handles_;
  }

  void add_asset_handle(this UiState& self,
                        backtest::AssetStoreHandle asset_handle) noexcept
  {
    self.asset_handles_.push_back(asset_handle);
  }

  void remove_asset_handle(this UiState& self,
                           backtest::AssetStoreHandle asset_handle) noexcept
  {
    auto& asset_handles = self.asset_handles_;
    const auto it = std::ranges::find(asset_handles, asset_handle);
    if(it != asset_handles.end()) {
      asset_handles.erase(it);
    }
  }

  auto strategy_handles(this const UiState& self) noexcept
   -> const std::vector<backtest::StrategyStoreHandle>&
  {
    return self.strategy_handles_;
  }

  void
  add_strategy_handle(this UiState& self,
                      backtest::StrategyStoreHandle strategy_handle) noexcept
  {
    self.strategy_handles_.push_back(strategy_handle);
  }

  void
  remove_strategy_handle(this UiState& self,
                         backtest::StrategyStoreHandle strategy_handle) noexcept
  {
    auto& strategy_handles = self.strategy_handles_;
    const auto it = std::ranges::find(strategy_handles, strategy_handle);
    if(it != strategy_handles.end()) {
      strategy_handles.erase(it);
    }
  }

  auto market_handles(this const UiState& self) noexcept
   -> const std::vector<backtest::MarketStoreHandle>&
  {
    return self.market_handles_;
  }

  void add_market_handle(this UiState& self,
                         backtest::MarketStoreHandle market_handle) noexcept
  {
    self.market_handles_.push_back(market_handle);
  }

  void remove_market_handle(this UiState& self,
                            backtest::MarketStoreHandle market_handle) noexcept
  {
    auto& market_handles = self.market_handles_;
    const auto it = std::ranges::find(market_handles, market_handle);
    if(it != market_handles.end()) {
      market_handles.erase(it);
    }
  }

  auto broker_handles(this const UiState& self) noexcept
   -> const std::vector<backtest::BrokerStoreHandle>&
  {
    return self.broker_handles_;
  }

  void add_broker_handle(this UiState& self,
                         backtest::BrokerStoreHandle broker_handle) noexcept
  {
    self.broker_handles_.push_back(broker_handle);
  }

  void remove_broker_handle(this UiState& self,
                            backtest::BrokerStoreHandle broker_handle) noexcept
  {
    auto& broker_handles = self.broker_handles_;
    const auto it = std::ranges::find(broker_handles, broker_handle);
    if(it != broker_handles.end()) {
      broker_handles.erase(it);
    }
  }

  auto profile_handles(this const UiState& self) noexcept
   -> const std::vector<backtest::ProfileStoreHandle>&
  {
    return self.profile_handles_;
  }

  void add_profile_handle(this UiState& self,
                          backtest::ProfileStoreHandle profile_handle) noexcept
  {
    self.profile_handles_.push_back(profile_handle);
  }

  void
  remove_profile_handle(this UiState& self,
                        backtest::ProfileStoreHandle profile_handle) noexcept
  {
    auto& profile_handles = self.profile_handles_;
    const auto it = std::ranges::find(profile_handles, profile_handle);
    if(it != profile_handles.end()) {
      profile_handles.erase(it);
    }
  }

private:
  std::string imgui_ini_settings_;

  backtest::BacktestStoreHandle selected_backtest_handle_;

  std::vector<backtest::BacktestStoreHandle> backtest_handles_;
  std::vector<backtest::AssetStoreHandle> asset_handles_;
  std::vector<backtest::StrategyStoreHandle> strategy_handles_;
  std::vector<backtest::MarketStoreHandle> market_handles_;
  std::vector<backtest::BrokerStoreHandle> broker_handles_;
  std::vector<backtest::ProfileStoreHandle> profile_handles_;
};

} // namespace pludux::apps
