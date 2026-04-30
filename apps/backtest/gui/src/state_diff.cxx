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

export module pludux.apps.backtest:state_diff;

import pludux.backtest;

import :ui_state;
import :application_state;

export namespace pludux::apps {

class StateDiff {
public:
  StateDiff(
   UiState ui_state,
   backtest::StoreDescriptor store_descriptor,
   Patch<backtest::Backtest> backtest_patch,
   Patch<backtest::Asset> asset_patch,
   Patch<backtest::Strategy> strategy_patch,
   Patch<backtest::Market> market_patch,
   Patch<backtest::Broker> broker_patch,
   Patch<backtest::Profile> profile_patch,
   Patch<std::vector<backtest::BacktestSummary>> backtest_summaries_patch,
   Patch<SeriesResultsCollector> series_results_patch)
  : ui_state_{std::move(ui_state)}
  , store_descriptor_{std::move(store_descriptor)}
  , backtest_patch_{std::move(backtest_patch)}
  , asset_patch_{std::move(asset_patch)}
  , strategy_patch_{std::move(strategy_patch)}
  , market_patch_{std::move(market_patch)}
  , broker_patch_{std::move(broker_patch)}
  , profile_patch_{std::move(profile_patch)}
  , backtest_summaries_patch_{std::move(backtest_summaries_patch)}
  , series_results_patch_{std::move(series_results_patch)}
  {
  }

  auto apply(this const StateDiff& self, const ApplicationState& old_state)
   -> ApplicationState
  {
    const auto& store = old_state.store();

    const auto& store_arena = store.arena();
    auto backtests = self.backtest_patch_.apply(store_arena.backtests());
    auto assets = self.asset_patch_.apply(store_arena.assets());
    auto strategies = self.strategy_patch_.apply(store_arena.strategies());
    auto markets = self.market_patch_.apply(store_arena.markets());
    auto brokers = self.broker_patch_.apply(store_arena.brokers());
    auto profiles = self.profile_patch_.apply(store_arena.profiles());
    auto backtest_summaries =
     self.backtest_summaries_patch_.apply(store_arena.backtest_summaries());
    auto series_results =
     self.series_results_patch_.apply(store_arena.series_results());

    return ApplicationState{
     backtest::Store{self.store_descriptor_,
                     backtest::StoreArena{std::move(backtests),
                                          std::move(assets),
                                          std::move(strategies),
                                          std::move(markets),
                                          std::move(brokers),
                                          std::move(profiles),
                                          std::move(backtest_summaries),
                                          std::move(series_results)}},
     self.ui_state_};
  }

private:
  UiState ui_state_;
  backtest::StoreDescriptor store_descriptor_;

  Patch<backtest::Backtest> backtest_patch_;
  Patch<backtest::Asset> asset_patch_;
  Patch<backtest::Strategy> strategy_patch_;
  Patch<backtest::Market> market_patch_;
  Patch<backtest::Broker> broker_patch_;
  Patch<backtest::Profile> profile_patch_;
  Patch<std::vector<backtest::BacktestSummary>> backtest_summaries_patch_;
  Patch<SeriesResultsCollector> series_results_patch_;
};

auto create_state_diff(const ApplicationState& old_state,
                       const ApplicationState& new_state) -> StateDiff
{
  const auto& old_store = old_state.store();
  const auto& new_store = new_state.store();

  const auto& old_store_arena = old_store.arena();
  const auto& new_store_arena = new_store.arena();

  auto backtest_patch =
   diff(old_store_arena.backtests(), new_store_arena.backtests());
  auto asset_patch = diff(old_store_arena.assets(), new_store_arena.assets());
  auto strategy_patch =
   diff(old_store_arena.strategies(), new_store_arena.strategies());
  auto market_patch =
   diff(old_store_arena.markets(), new_store_arena.markets());
  auto broker_patch =
   diff(old_store_arena.brokers(), new_store_arena.brokers());
  auto profile_patch =
   diff(old_store_arena.profiles(), new_store_arena.profiles());
  auto backtest_summaries_patch = diff(old_store_arena.backtest_summaries(),
                                       new_store_arena.backtest_summaries());
  auto series_results_patch =
   diff(old_store_arena.series_results(), new_store_arena.series_results());

  return StateDiff{new_state.ui_state(),
                   new_store.descriptor(),
                   std::move(backtest_patch),
                   std::move(asset_patch),
                   std::move(strategy_patch),
                   std::move(market_patch),
                   std::move(broker_patch),
                   std::move(profile_patch),
                   std::move(backtest_summaries_patch),
                   std::move(series_results_patch)};
}

} // namespace pludux::apps
