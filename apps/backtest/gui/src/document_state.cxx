module;

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <utility>
#include <vector>

export module pludux.apps.backtest.document_state;

import pludux.backtest;

export namespace pludux::apps {

class DocumentState {
public:
  DocumentState() = default;

  DocumentState(std::vector<backtest::PortfolioStoreHandle> portfolio_handles,
                std::vector<backtest::BacktestStoreHandle> backtest_handles,
                std::vector<backtest::AssetStoreHandle> asset_handles,
                std::vector<backtest::StrategyStoreHandle> strategy_handles,
                std::vector<backtest::MarketStoreHandle> market_handles,
                std::vector<backtest::BrokerStoreHandle> broker_handles,
                std::vector<backtest::ProfileStoreHandle> profile_handles)
  : portfolio_handles_{std::move(portfolio_handles)}
  , backtest_handles_{std::move(backtest_handles)}
  , asset_handles_{std::move(asset_handles)}
  , strategy_handles_{std::move(strategy_handles)}
  , market_handles_{std::move(market_handles)}
  , broker_handles_{std::move(broker_handles)}
  , profile_handles_{std::move(profile_handles)}
  {
  }

  auto operator==(const DocumentState&) const noexcept -> bool = default;

#define PLUDUX_DOCUMENT_STATE_HANDLES(Resource, resource)                     \
  auto resource##_handles(this const DocumentState& self) noexcept            \
   -> const std::vector<backtest::Resource##StoreHandle>&                     \
  {                                                                           \
    return self.resource##_handles_;                                          \
  }                                                                           \
                                                                              \
  auto resource##_handles(this DocumentState& self) noexcept                  \
   -> std::vector<backtest::Resource##StoreHandle>&                           \
  {                                                                           \
    return self.resource##_handles_;                                          \
  }                                                                           \
                                                                              \
  void reorder_##resource##_handle(this DocumentState& self,                  \
                                   std::size_t from_index,                    \
                                   std::size_t to_index) noexcept             \
  {                                                                           \
    auto& handles = self.resource##_handles_;                                 \
    if(from_index >= handles.size() || to_index >= handles.size()) {          \
      return;                                                                 \
    }                                                                         \
    const auto handle = handles[from_index];                                  \
    handles.erase(handles.begin() + from_index);                              \
    handles.insert(handles.begin() + to_index, handle);                       \
  }                                                                           \
                                                                              \
  void add_##resource##_handle(                                               \
   this DocumentState& self, backtest::Resource##StoreHandle handle) noexcept \
  {                                                                           \
    self.resource##_handles_.push_back(handle);                               \
  }                                                                           \
                                                                              \
  void remove_##resource##_handle(                                            \
   this DocumentState& self, backtest::Resource##StoreHandle handle) noexcept \
  {                                                                           \
    auto& handles = self.resource##_handles_;                                 \
    const auto found = std::ranges::find(handles, handle);                    \
    if(found != handles.end()) {                                              \
      handles.erase(found);                                                   \
    }                                                                         \
  }

  PLUDUX_DOCUMENT_STATE_HANDLES(Portfolio, portfolio)
  PLUDUX_DOCUMENT_STATE_HANDLES(Backtest, backtest)
  PLUDUX_DOCUMENT_STATE_HANDLES(Asset, asset)
  PLUDUX_DOCUMENT_STATE_HANDLES(Strategy, strategy)
  PLUDUX_DOCUMENT_STATE_HANDLES(Market, market)
  PLUDUX_DOCUMENT_STATE_HANDLES(Broker, broker)
  PLUDUX_DOCUMENT_STATE_HANDLES(Profile, profile)

#undef PLUDUX_DOCUMENT_STATE_HANDLES

private:
  std::vector<backtest::PortfolioStoreHandle> portfolio_handles_;
  std::vector<backtest::BacktestStoreHandle> backtest_handles_;
  std::vector<backtest::AssetStoreHandle> asset_handles_;
  std::vector<backtest::StrategyStoreHandle> strategy_handles_;
  std::vector<backtest::MarketStoreHandle> market_handles_;
  std::vector<backtest::BrokerStoreHandle> broker_handles_;
  std::vector<backtest::ProfileStoreHandle> profile_handles_;
};

} // namespace pludux::apps
