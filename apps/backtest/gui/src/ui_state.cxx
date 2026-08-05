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
import pludux.apps.backtest.portfolio_backtest_selections;

export namespace pludux::apps {

class UiState {
public:
  UiState() = default;

  UiState(std::string imgui_ini_settings,
          backtest::PortfolioStoreHandle selected_portfolio_handle,
          std::vector<backtest::PortfolioStoreHandle> portfolio_handles,
          PortfolioBacktestSelections portfolio_backtest_selections,
          backtest::BacktestStoreHandle selected_backtest_handle,
          std::vector<backtest::BacktestStoreHandle> backtest_handles,
          std::vector<backtest::AssetStoreHandle> asset_handles,
          std::vector<backtest::StrategyStoreHandle> strategy_handles,
          std::vector<backtest::MarketStoreHandle> market_handles,
          std::vector<backtest::BrokerStoreHandle> broker_handles,
          std::vector<backtest::ProfileStoreHandle> profile_handles)
  : imgui_ini_settings_{std::move(imgui_ini_settings)}
  , selected_portfolio_handle_{selected_portfolio_handle}
  , portfolio_handles_{std::move(portfolio_handles)}
  , portfolio_backtest_selections_{std::move(portfolio_backtest_selections)}
  , selected_backtest_handle_{std::move(selected_backtest_handle)}
  , backtest_handles_{std::move(backtest_handles)}
  , asset_handles_{std::move(asset_handles)}
  , strategy_handles_{std::move(strategy_handles)}
  , market_handles_{std::move(market_handles)}
  , broker_handles_{std::move(broker_handles)}
  , profile_handles_{std::move(profile_handles)}
  {
  }

  auto selected_portfolio_handle(this const UiState& self) noexcept
   -> backtest::PortfolioStoreHandle
  {
    return self.selected_portfolio_handle_;
  }

  void selected_portfolio_handle(this UiState& self,
                                 backtest::PortfolioStoreHandle handle) noexcept
  {
    self.selected_portfolio_handle_ = handle;
  }

  auto portfolio_handles(this const UiState& self) noexcept
   -> const std::vector<backtest::PortfolioStoreHandle>&
  {
    return self.portfolio_handles_;
  }

  auto portfolio_handles(this UiState& self) noexcept
   -> std::vector<backtest::PortfolioStoreHandle>&
  {
    return self.portfolio_handles_;
  }

  auto portfolio_backtest_selections(this const UiState& self) noexcept
   -> const PortfolioBacktestSelections&
  {
    return self.portfolio_backtest_selections_;
  }

  auto portfolio_backtest_selections(this UiState& self) noexcept
   -> PortfolioBacktestSelections&
  {
    return self.portfolio_backtest_selections_;
  }

  void reorder_portfolio_handle(this UiState& self,
                                std::size_t from_index,
                                std::size_t to_index) noexcept
  {
    if(from_index >= self.portfolio_handles_.size() ||
       to_index >= self.portfolio_handles_.size()) {
      return;
    }
    const auto handle = self.portfolio_handles_[from_index];
    self.portfolio_handles_.erase(self.portfolio_handles_.begin() + from_index);
    self.portfolio_handles_.insert(self.portfolio_handles_.begin() + to_index,
                                   handle);
  }

  void add_portfolio_handle(this UiState& self,
                            backtest::PortfolioStoreHandle handle) noexcept
  {
    self.portfolio_handles_.push_back(handle);
  }

  void remove_portfolio_handle(this UiState& self,
                               backtest::PortfolioStoreHandle handle) noexcept
  {
    const auto found = std::ranges::find(self.portfolio_handles_, handle);
    if(found != self.portfolio_handles_.end()) {
      self.portfolio_handles_.erase(found);
    }
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

  void reorder_backtest_handle(this UiState& self,
                               std::size_t from_index,
                               std::size_t to_index) noexcept
  {
    auto& backtest_handles = self.backtest_handles_;
    if(from_index < backtest_handles.size() &&
       to_index < backtest_handles.size()) {
      const auto handle = backtest_handles[from_index];
      backtest_handles.erase(backtest_handles.begin() + from_index);
      backtest_handles.insert(backtest_handles.begin() + to_index, handle);
    }
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

  void reorder_asset_handle(this UiState& self,
                            std::size_t from_index,
                            std::size_t to_index) noexcept
  {
    auto& asset_handles = self.asset_handles_;
    if(from_index < asset_handles.size() && to_index < asset_handles.size()) {
      const auto handle = asset_handles[from_index];
      asset_handles.erase(asset_handles.begin() + from_index);
      asset_handles.insert(asset_handles.begin() + to_index, handle);
    }
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

  void reorder_strategy_handle(this UiState& self,
                               std::size_t from_index,
                               std::size_t to_index) noexcept
  {
    auto& strategy_handles = self.strategy_handles_;
    if(from_index < strategy_handles.size() &&
       to_index < strategy_handles.size()) {
      const auto handle = strategy_handles[from_index];
      strategy_handles.erase(strategy_handles.begin() + from_index);
      strategy_handles.insert(strategy_handles.begin() + to_index, handle);
    }
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

  void reorder_market_handle(this UiState& self,
                             std::size_t from_index,
                             std::size_t to_index) noexcept
  {
    auto& market_handles = self.market_handles_;
    if(from_index < market_handles.size() && to_index < market_handles.size()) {
      const auto handle = market_handles[from_index];
      market_handles.erase(market_handles.begin() + from_index);
      market_handles.insert(market_handles.begin() + to_index, handle);
    }
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

  void reorder_broker_handle(this UiState& self,
                             std::size_t from_index,
                             std::size_t to_index) noexcept
  {
    auto& broker_handles = self.broker_handles_;
    if(from_index < broker_handles.size() && to_index < broker_handles.size()) {
      const auto handle = broker_handles[from_index];
      broker_handles.erase(broker_handles.begin() + from_index);
      broker_handles.insert(broker_handles.begin() + to_index, handle);
    }
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

  void reorder_profile_handle(this UiState& self,
                              std::size_t from_index,
                              std::size_t to_index) noexcept
  {
    auto& profile_handles = self.profile_handles_;
    if(from_index < profile_handles.size() &&
       to_index < profile_handles.size()) {
      const auto handle = profile_handles[from_index];
      profile_handles.erase(profile_handles.begin() + from_index);
      profile_handles.insert(profile_handles.begin() + to_index, handle);
    }
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
  backtest::PortfolioStoreHandle selected_portfolio_handle_;
  std::vector<backtest::PortfolioStoreHandle> portfolio_handles_;
  PortfolioBacktestSelections portfolio_backtest_selections_;
  backtest::BacktestStoreHandle selected_backtest_handle_;

  std::vector<backtest::BacktestStoreHandle> backtest_handles_;
  std::vector<backtest::AssetStoreHandle> asset_handles_;
  std::vector<backtest::StrategyStoreHandle> strategy_handles_;
  std::vector<backtest::MarketStoreHandle> market_handles_;
  std::vector<backtest::BrokerStoreHandle> broker_handles_;
  std::vector<backtest::ProfileStoreHandle> profile_handles_;
};

} // namespace pludux::apps
