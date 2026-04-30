module;

#include <optional>
#include <utility>
#include <vector>

export module pludux.backtest:store;

import pludux;

import :store_descriptor;
import :store_arena;

export namespace pludux::backtest {

class Store {
public:
  Store() = default;

  Store(StoreDescriptor store_descriptor, StoreArena store_arena)
  : descriptor_{std::move(store_descriptor)}
  , arena_{std::move(store_arena)}
  {
  }

  auto descriptor(this const Store& self) noexcept -> const StoreDescriptor&
  {
    return self.descriptor_;
  }

  auto descriptor(this Store& self) noexcept -> StoreDescriptor&
  {
    return self.descriptor_;
  }

  auto arena(this const Store& self) noexcept -> const StoreArena&
  {
    return self.arena_;
  }

  auto arena(this Store& self) noexcept -> StoreArena&
  {
    return self.arena_;
  }

  auto add_backtest(this Store& self, Backtest backtest)
   -> std::optional<BacktestStoreHandle>
  {
    auto& backtests = self.arena_.backtests();
    auto& backtest_resolver = self.descriptor_.backtest_store_data_resolver();

    return backtest_resolver.add(backtests, std::move(backtest));
  }

  auto get_backtest(this const Store& self, BacktestStoreHandle handle) noexcept
   -> const Backtest&
  {
    const auto& backtests = self.arena_.backtests();
    const auto& backtest_resolver =
     self.descriptor_.backtest_store_data_resolver();
    return backtest_resolver.get(backtests, handle);
  }

  auto get_backtest(this Store& self, BacktestStoreHandle handle) noexcept
   -> Backtest&
  {
    auto& backtests = self.arena_.backtests();
    auto& backtest_resolver = self.descriptor_.backtest_store_data_resolver();
    return backtest_resolver.get(backtests, handle);
  }

  auto get_backtest_if_present(this const Store& self,
                               BacktestStoreHandle handle) noexcept
   -> const Backtest*
  {
    const auto& backtests = self.arena_.backtests();
    const auto& backtest_resolver =
     self.descriptor_.backtest_store_data_resolver();
    return backtest_resolver.get_if_present(backtests, handle);
  }

  auto get_backtest_if_present(this Store& self,
                               BacktestStoreHandle handle) noexcept -> Backtest*
  {
    auto& backtests = self.arena_.backtests();
    auto& backtest_resolver = self.descriptor_.backtest_store_data_resolver();
    return backtest_resolver.get_if_present(backtests, handle);
  }

  auto update_backtest(this Store& self,
                       BacktestStoreHandle handle,
                       Backtest backtest) -> bool
  {
    auto& backtests = self.arena_.backtests();
    auto& backtest_resolver = self.descriptor_.backtest_store_data_resolver();
    return backtest_resolver.update(backtests, handle, std::move(backtest));
  }

  auto remove_backtest(this Store& self, BacktestStoreHandle handle) -> bool
  {
    auto& backtests = self.arena_.backtests();
    auto& backtest_resolver = self.descriptor_.backtest_store_data_resolver();
    return backtest_resolver.remove(backtests, handle);
  }

  auto add_asset(this Store& self, Asset asset)
   -> std::optional<AssetStoreHandle>
  {
    auto& assets = self.arena_.assets();
    auto& asset_resolver = self.descriptor_.asset_store_data_resolver();

    return asset_resolver.add(assets, std::move(asset));
  }

  auto get_asset(this const Store& self, AssetStoreHandle handle) noexcept
   -> const Asset&
  {
    const auto& assets = self.arena_.assets();
    const auto& asset_resolver = self.descriptor_.asset_store_data_resolver();
    return asset_resolver.get(assets, handle);
  }

  auto get_asset(this Store& self, AssetStoreHandle handle) noexcept -> Asset&
  {
    auto& assets = self.arena_.assets();
    auto& asset_resolver = self.descriptor_.asset_store_data_resolver();
    return asset_resolver.get(assets, handle);
  }

  auto get_asset_if_present(this const Store& self,
                            AssetStoreHandle handle) noexcept -> const Asset*
  {
    const auto& assets = self.arena_.assets();
    const auto& asset_resolver = self.descriptor_.asset_store_data_resolver();
    return asset_resolver.get_if_present(assets, handle);
  }

  auto get_asset_if_present(this Store& self, AssetStoreHandle handle) noexcept
   -> Asset*
  {
    auto& assets = self.arena_.assets();
    auto& asset_resolver = self.descriptor_.asset_store_data_resolver();
    return asset_resolver.get_if_present(assets, handle);
  }

  auto update_asset(this Store& self, AssetStoreHandle handle, Asset asset)
   -> bool
  {
    auto& assets = self.arena_.assets();
    auto& asset_resolver = self.descriptor_.asset_store_data_resolver();
    return asset_resolver.update(assets, handle, std::move(asset));
  }

  auto remove_asset(this Store& self, AssetStoreHandle handle) -> bool
  {
    auto& assets = self.arena_.assets();
    auto& asset_resolver = self.descriptor_.asset_store_data_resolver();
    return asset_resolver.remove(assets, handle);
  }

  auto add_strategy(this Store& self, Strategy strategy)
   -> std::optional<StrategyStoreHandle>
  {
    auto& strategies = self.arena_.strategies();
    auto& strategy_resolver = self.descriptor_.strategy_store_data_resolver();

    return strategy_resolver.add(strategies, std::move(strategy));
  }

  auto get_strategy(this const Store& self, StrategyStoreHandle handle) noexcept
   -> const Strategy&
  {
    const auto& strategies = self.arena_.strategies();
    const auto& strategy_resolver =
     self.descriptor_.strategy_store_data_resolver();
    return strategy_resolver.get(strategies, handle);
  }

  auto get_strategy(this Store& self, StrategyStoreHandle handle) noexcept
   -> Strategy&
  {
    auto& strategies = self.arena_.strategies();
    auto& strategy_resolver = self.descriptor_.strategy_store_data_resolver();
    return strategy_resolver.get(strategies, handle);
  }

  auto get_strategy_if_present(this const Store& self,
                               StrategyStoreHandle handle) noexcept
   -> const Strategy*
  {
    const auto& strategies = self.arena_.strategies();
    const auto& strategy_resolver =
     self.descriptor_.strategy_store_data_resolver();
    return strategy_resolver.get_if_present(strategies, handle);
  }

  auto get_strategy_if_present(this Store& self,
                               StrategyStoreHandle handle) noexcept -> Strategy*
  {
    auto& strategies = self.arena_.strategies();
    auto& strategy_resolver = self.descriptor_.strategy_store_data_resolver();
    return strategy_resolver.get_if_present(strategies, handle);
  }

  auto update_strategy(this Store& self,
                       StrategyStoreHandle handle,
                       Strategy strategy) -> bool
  {
    auto& strategies = self.arena_.strategies();
    auto& strategy_resolver = self.descriptor_.strategy_store_data_resolver();
    return strategy_resolver.update(strategies, handle, std::move(strategy));
  }

  auto remove_strategy(this Store& self, StrategyStoreHandle handle) -> bool
  {
    auto& strategies = self.arena_.strategies();
    auto& strategy_resolver = self.descriptor_.strategy_store_data_resolver();
    return strategy_resolver.remove(strategies, handle);
  }

  auto add_market(this Store& self, Market market)
   -> std::optional<MarketStoreHandle>
  {
    auto& markets = self.arena_.markets();
    auto& market_resolver = self.descriptor_.market_store_data_resolver();

    return market_resolver.add(markets, std::move(market));
  }

  auto get_market(this const Store& self, MarketStoreHandle handle) noexcept
   -> const Market&
  {
    const auto& markets = self.arena_.markets();
    const auto& market_resolver = self.descriptor_.market_store_data_resolver();
    return market_resolver.get(markets, handle);
  }

  auto get_market(this Store& self, MarketStoreHandle handle) noexcept
   -> Market&
  {
    auto& markets = self.arena_.markets();
    auto& market_resolver = self.descriptor_.market_store_data_resolver();
    return market_resolver.get(markets, handle);
  }

  auto get_market_if_present(this const Store& self,
                             MarketStoreHandle handle) noexcept -> const Market*
  {
    const auto& markets = self.arena_.markets();
    const auto& market_resolver = self.descriptor_.market_store_data_resolver();
    return market_resolver.get_if_present(markets, handle);
  }

  auto get_market_if_present(this Store& self,
                             MarketStoreHandle handle) noexcept -> Market*
  {
    auto& markets = self.arena_.markets();
    auto& market_resolver = self.descriptor_.market_store_data_resolver();
    return market_resolver.get_if_present(markets, handle);
  }

  auto update_market(this Store& self, MarketStoreHandle handle, Market market)
   -> bool
  {
    auto& markets = self.arena_.markets();
    auto& market_resolver = self.descriptor_.market_store_data_resolver();
    return market_resolver.update(markets, handle, std::move(market));
  }

  auto remove_market(this Store& self, MarketStoreHandle handle) -> bool
  {
    auto& markets = self.arena_.markets();
    auto& market_resolver = self.descriptor_.market_store_data_resolver();
    return market_resolver.remove(markets, handle);
  }

  auto add_broker(this Store& self, Broker broker)
   -> std::optional<BrokerStoreHandle>
  {
    auto& brokers = self.arena_.brokers();
    auto& broker_resolver = self.descriptor_.broker_store_data_resolver();

    return broker_resolver.add(brokers, std::move(broker));
  }

  auto get_broker(this const Store& self, BrokerStoreHandle handle) noexcept
   -> const Broker&
  {
    const auto& brokers = self.arena_.brokers();
    const auto& broker_resolver = self.descriptor_.broker_store_data_resolver();
    return broker_resolver.get(brokers, handle);
  }

  auto get_broker(this Store& self, BrokerStoreHandle handle) noexcept
   -> Broker&
  {
    auto& brokers = self.arena_.brokers();
    auto& broker_resolver = self.descriptor_.broker_store_data_resolver();
    return broker_resolver.get(brokers, handle);
  }

  auto get_broker_if_present(this const Store& self,
                             BrokerStoreHandle handle) noexcept -> const Broker*
  {
    const auto& brokers = self.arena_.brokers();
    const auto& broker_resolver = self.descriptor_.broker_store_data_resolver();
    return broker_resolver.get_if_present(brokers, handle);
  }

  auto get_broker_if_present(this Store& self,
                             BrokerStoreHandle handle) noexcept -> Broker*
  {
    auto& brokers = self.arena_.brokers();
    auto& broker_resolver = self.descriptor_.broker_store_data_resolver();
    return broker_resolver.get_if_present(brokers, handle);
  }

  auto update_broker(this Store& self, BrokerStoreHandle handle, Broker broker)
   -> bool
  {
    auto& brokers = self.arena_.brokers();
    auto& broker_resolver = self.descriptor_.broker_store_data_resolver();
    return broker_resolver.update(brokers, handle, std::move(broker));
  }

  auto remove_broker(this Store& self, BrokerStoreHandle handle) -> bool
  {
    auto& brokers = self.arena_.brokers();
    auto& broker_resolver = self.descriptor_.broker_store_data_resolver();
    return broker_resolver.remove(brokers, handle);
  }

  auto add_profile(this Store& self, Profile profile)
   -> std::optional<ProfileStoreHandle>
  {
    auto& profiles = self.arena_.profiles();
    auto& profile_resolver = self.descriptor_.profile_store_data_resolver();

    return profile_resolver.add(profiles, std::move(profile));
  }

  auto get_profile(this const Store& self, ProfileStoreHandle handle) noexcept
   -> const Profile&
  {
    const auto& profiles = self.arena_.profiles();
    const auto& profile_resolver =
     self.descriptor_.profile_store_data_resolver();
    return profile_resolver.get(profiles, handle);
  }

  auto get_profile(this Store& self, ProfileStoreHandle handle) noexcept
   -> Profile&
  {
    auto& profiles = self.arena_.profiles();
    auto& profile_resolver = self.descriptor_.profile_store_data_resolver();
    return profile_resolver.get(profiles, handle);
  }

  auto get_profile_if_present(this const Store& self,
                              ProfileStoreHandle handle) noexcept
   -> const Profile*
  {
    const auto& profiles = self.arena_.profiles();
    const auto& profile_resolver =
     self.descriptor_.profile_store_data_resolver();
    return profile_resolver.get_if_present(profiles, handle);
  }

  auto get_profile_if_present(this Store& self,
                              ProfileStoreHandle handle) noexcept -> Profile*
  {
    auto& profiles = self.arena_.profiles();
    auto& profile_resolver = self.descriptor_.profile_store_data_resolver();
    return profile_resolver.get_if_present(profiles, handle);
  }

  auto update_profile(this Store& self,
                      ProfileStoreHandle handle,
                      Profile profile) -> bool
  {
    auto& profiles = self.arena_.profiles();
    auto& profile_resolver = self.descriptor_.profile_store_data_resolver();
    return profile_resolver.update(profiles, handle, std::move(profile));
  }

  auto remove_profile(this Store& self, ProfileStoreHandle handle) -> bool
  {
    auto& profiles = self.arena_.profiles();
    auto& profile_resolver = self.descriptor_.profile_store_data_resolver();
    return profile_resolver.remove(profiles, handle);
  }

  auto add_backtest_summaries(this Store& self,
                              BacktestStoreHandle handle,
                              std::vector<BacktestSummary> summaries) -> bool
  {
    auto& backtest_summaries = self.arena_.backtest_summaries();
    auto& backtest_summaries_resolver =
     self.descriptor_.backtest_summaries_store_data_resolver();

    return backtest_summaries_resolver.add(
     backtest_summaries, handle, std::move(summaries));
  }

  auto get_backtest_summaries(this const Store& self,
                              BacktestStoreHandle handle) noexcept
   -> const std::vector<BacktestSummary>&
  {
    const auto& backtest_summaries = self.arena_.backtest_summaries();
    const auto& backtest_summaries_resolver =
     self.descriptor_.backtest_summaries_store_data_resolver();

    return backtest_summaries_resolver.get(backtest_summaries, handle);
  }

  auto get_backtest_summaries(this Store& self,
                              BacktestStoreHandle handle) noexcept
   -> std::vector<BacktestSummary>&
  {
    auto& backtest_summaries = self.arena_.backtest_summaries();
    auto& backtest_summaries_resolver =
     self.descriptor_.backtest_summaries_store_data_resolver();

    return backtest_summaries_resolver.get(backtest_summaries, handle);
  }

  auto get_backtest_summaries_if_present(this const Store& self,
                                         BacktestStoreHandle handle) noexcept
   -> const std::vector<BacktestSummary>*
  {
    const auto& backtest_summaries = self.arena_.backtest_summaries();
    const auto& backtest_summaries_resolver =
     self.descriptor_.backtest_summaries_store_data_resolver();

    return backtest_summaries_resolver.get_if_present(backtest_summaries,
                                                      handle);
  }

  auto get_backtest_summaries_if_present(this Store& self,
                                         BacktestStoreHandle handle) noexcept
   -> std::vector<BacktestSummary>*
  {
    auto& backtest_summaries = self.arena_.backtest_summaries();
    auto& backtest_summaries_resolver =
     self.descriptor_.backtest_summaries_store_data_resolver();

    return backtest_summaries_resolver.get_if_present(backtest_summaries,
                                                      handle);
  }

  auto update_backtest_summaries(this Store& self,
                                 BacktestStoreHandle handle,
                                 std::vector<BacktestSummary> summaries) -> bool
  {
    auto& backtest_summaries = self.arena_.backtest_summaries();
    auto& backtest_summaries_resolver =
     self.descriptor_.backtest_summaries_store_data_resolver();

    return backtest_summaries_resolver.update(
     backtest_summaries, handle, std::move(summaries));
  }

  auto remove_backtest_summaries(this Store& self, BacktestStoreHandle handle)
   -> bool
  {
    auto& backtest_summaries = self.arena_.backtest_summaries();
    auto& backtest_summaries_resolver =
     self.descriptor_.backtest_summaries_store_data_resolver();

    return backtest_summaries_resolver.remove(backtest_summaries, handle);
  }

  auto add_series_results(this Store& self,
                          BacktestStoreHandle handle,
                          SeriesResultsCollector series_result) -> bool
  {
    auto& series_results = self.arena_.series_results();
    auto& series_results_resolver =
     self.descriptor_.series_results_store_data_resolver();

    return series_results_resolver.add(
     series_results, handle, std::move(series_result));
  }

  auto get_series_results(this const Store& self,
                          BacktestStoreHandle handle) noexcept
   -> const SeriesResultsCollector&
  {
    const auto& series_results = self.arena_.series_results();
    const auto& series_results_resolver =
     self.descriptor_.series_results_store_data_resolver();

    return series_results_resolver.get(series_results, handle);
  }

  auto get_series_results(this Store& self, BacktestStoreHandle handle) noexcept
   -> SeriesResultsCollector&
  {
    auto& series_results = self.arena_.series_results();
    auto& series_results_resolver =
     self.descriptor_.series_results_store_data_resolver();

    return series_results_resolver.get(series_results, handle);
  }

  auto get_series_results_if_present(this const Store& self,
                                     BacktestStoreHandle handle) noexcept
   -> const SeriesResultsCollector*
  {
    const auto& series_results = self.arena_.series_results();
    const auto& series_results_resolver =
     self.descriptor_.series_results_store_data_resolver();

    return series_results_resolver.get_if_present(series_results, handle);
  }

  auto get_series_results_if_present(this Store& self,
                                     BacktestStoreHandle handle) noexcept
   -> SeriesResultsCollector*
  {
    auto& series_results = self.arena_.series_results();
    auto& series_results_resolver =
     self.descriptor_.series_results_store_data_resolver();

    return series_results_resolver.get_if_present(series_results, handle);
  }

  auto update_series_results(this Store& self,
                             BacktestStoreHandle handle,
                             SeriesResultsCollector series_result) -> bool
  {
    auto& series_results = self.arena_.series_results();
    auto& series_results_resolver =
     self.descriptor_.series_results_store_data_resolver();

    return series_results_resolver.update(
     series_results, handle, std::move(series_result));
  }

  auto remove_series_results(this Store& self, BacktestStoreHandle handle)
   -> bool
  {
    auto& series_results = self.arena_.series_results();
    auto& series_results_resolver =
     self.descriptor_.series_results_store_data_resolver();

    return series_results_resolver.remove(series_results, handle);
  }

private:
  StoreDescriptor descriptor_;
  StoreArena arena_;
};

} // namespace pludux::backtest
