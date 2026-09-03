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

export module pludux.apps.backtest.state_diff;

import pludux.backtest;

import pludux.apps.backtest.document_state;
import pludux.apps.backtest.application_state;

export namespace pludux::apps {

class StateDiff {
public:
  StateDiff(DocumentState document_state,
            backtest::StoreDescriptor store_descriptor,
             Patch<backtest::System> system_patch,
            Patch<backtest::Portfolio> portfolio_patch,
            Patch<backtest::Asset> asset_patch,
            Patch<backtest::Watchlist> watchlist_patch,
             Patch<backtest::Model> model_patch,
            Patch<backtest::Market> market_patch,
            Patch<backtest::Broker> broker_patch,
             Patch<backtest::Profile> profile_patch,
             Patch<backtest::Strategy> strategy_patch,
            Patch<backtest::PortfolioResults> portfolio_results_patch)
  : document_state_{std::move(document_state)}
  , store_descriptor_{std::move(store_descriptor)}
    , system_patch_{std::move(system_patch)}
  , portfolio_patch_{std::move(portfolio_patch)}
  , asset_patch_{std::move(asset_patch)}
  , watchlist_patch_{std::move(watchlist_patch)}
   , model_patch_{std::move(model_patch)}
  , market_patch_{std::move(market_patch)}
  , broker_patch_{std::move(broker_patch)}
   , profile_patch_{std::move(profile_patch)}
   , strategy_patch_{std::move(strategy_patch)}
  , portfolio_results_patch_{std::move(portfolio_results_patch)}
  {
  }

  auto empty(this const StateDiff& self,
             const ApplicationState& source_state) noexcept -> bool
  {
    const auto changed = [](const auto& patch) {
      return patch.inserted_count() != 0 || patch.deleted_count() != 0;
    };
    return self.document_state_ == source_state.document_state() &&
             !changed(self.system_patch_) && !changed(self.portfolio_patch_) &&
            !changed(self.asset_patch_) && !changed(self.model_patch_) &&
           !changed(self.watchlist_patch_) && !changed(self.market_patch_) &&
            !changed(self.broker_patch_) && !changed(self.profile_patch_) &&
            !changed(self.strategy_patch_) &&
           !changed(self.portfolio_results_patch_);
  }

  auto apply(this const StateDiff& self, const ApplicationState& old_state)
   -> ApplicationState
  {
    const auto& store = old_state.store();

    const auto& store_arena = store.arena();
    auto systems = self.system_patch_.apply(store_arena.systems());
    auto portfolios = self.portfolio_patch_.apply(store_arena.portfolios());
    auto assets = self.asset_patch_.apply(store_arena.assets());
    auto watchlists = self.watchlist_patch_.apply(store_arena.watchlists());
    auto models = self.model_patch_.apply(store_arena.models());
    auto markets = self.market_patch_.apply(store_arena.markets());
    auto brokers = self.broker_patch_.apply(store_arena.brokers());
    auto profiles = self.profile_patch_.apply(store_arena.profiles());
    auto strategies = self.strategy_patch_.apply(store_arena.strategies());
    auto portfolio_results =
     self.portfolio_results_patch_.apply(store_arena.portfolio_results());

    return ApplicationState{
     backtest::Store{self.store_descriptor_,
      backtest::StoreArena{std::move(systems),
                                          std::move(portfolios),
                                          std::move(assets),
                                          std::move(watchlists),
                                           std::move(models),
                                          std::move(markets),
                                          std::move(brokers),
                                           std::move(profiles),
                                           std::move(strategies),
                                           std::move(portfolio_results)}},
     self.document_state_,
     old_state.view_state()};
  }

private:
  DocumentState document_state_;
  backtest::StoreDescriptor store_descriptor_;

  Patch<backtest::System> system_patch_;
  Patch<backtest::Portfolio> portfolio_patch_;
  Patch<backtest::Asset> asset_patch_;
  Patch<backtest::Watchlist> watchlist_patch_;
  Patch<backtest::Model> model_patch_;
  Patch<backtest::Market> market_patch_;
  Patch<backtest::Broker> broker_patch_;
  Patch<backtest::Profile> profile_patch_;
  Patch<backtest::Strategy> strategy_patch_;
  Patch<backtest::PortfolioResults> portfolio_results_patch_;
};

auto create_state_diff(const ApplicationState& old_state,
                       const ApplicationState& new_state) -> StateDiff
{
  const auto& old_store = old_state.store();
  const auto& new_store = new_state.store();

  const auto& old_store_arena = old_store.arena();
  const auto& new_store_arena = new_store.arena();

  auto system_patch = diff(old_store_arena.systems(), new_store_arena.systems());
  auto portfolio_patch =
   diff(old_store_arena.portfolios(), new_store_arena.portfolios());
  auto asset_patch = diff(old_store_arena.assets(), new_store_arena.assets());
  auto watchlist_patch =
   diff(old_store_arena.watchlists(), new_store_arena.watchlists());
  auto model_patch = diff(old_store_arena.models(), new_store_arena.models());
  auto market_patch =
   diff(old_store_arena.markets(), new_store_arena.markets());
  auto broker_patch =
   diff(old_store_arena.brokers(), new_store_arena.brokers());
  auto profile_patch =
   diff(old_store_arena.profiles(), new_store_arena.profiles());
  auto strategy_patch =
   diff(old_store_arena.strategies(), new_store_arena.strategies());
  auto portfolio_results_patch = diff(old_store_arena.portfolio_results(),
                                      new_store_arena.portfolio_results());

  return StateDiff{new_state.document_state(),
                   new_store.descriptor(),
                    std::move(system_patch),
                   std::move(portfolio_patch),
                   std::move(asset_patch),
                   std::move(watchlist_patch),
                    std::move(model_patch),
                   std::move(market_patch),
                   std::move(broker_patch),
                    std::move(profile_patch),
                    std::move(strategy_patch),
                   std::move(portfolio_results_patch)};
}

} // namespace pludux::apps
