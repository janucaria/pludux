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

  auto add_system(this Store& self, System system)
    -> std::optional<SystemStoreHandle>
  {
    auto& systems = self.arena_.systems();
    auto& system_resolver = self.descriptor_.system_store_data_resolver();

    return system_resolver.add(systems, std::move(system));
  }

  auto get_system(this const Store& self, SystemStoreHandle handle) noexcept
    -> const System&
  {
    const auto& systems = self.arena_.systems();
    const auto& system_resolver = self.descriptor_.system_store_data_resolver();
    return system_resolver.get(systems, handle);
  }

  auto get_system(this Store& self, SystemStoreHandle handle) noexcept
    -> System&
  {
    auto& systems = self.arena_.systems();
    auto& system_resolver = self.descriptor_.system_store_data_resolver();
    return system_resolver.get(systems, handle);
  }

  auto get_system_if_present(this const Store& self,
                             SystemStoreHandle handle) noexcept
    -> const System*
  {
    const auto& systems = self.arena_.systems();
    const auto& system_resolver = self.descriptor_.system_store_data_resolver();
    return system_resolver.get_if_present(systems, handle);
  }

  auto get_system_if_present(this Store& self,
                             SystemStoreHandle handle) noexcept -> System*
  {
    auto& systems = self.arena_.systems();
    auto& system_resolver = self.descriptor_.system_store_data_resolver();
    return system_resolver.get_if_present(systems, handle);
  }

  auto update_system(this Store& self,
                     SystemStoreHandle handle,
                     System system) -> bool
  {
    auto& systems = self.arena_.systems();
    auto& system_resolver = self.descriptor_.system_store_data_resolver();
    return system_resolver.update(systems, handle, std::move(system));
  }

  auto remove_system(this Store& self, SystemStoreHandle handle) -> bool
  {
    auto& systems = self.arena_.systems();
    auto& system_resolver = self.descriptor_.system_store_data_resolver();
    return system_resolver.remove(systems, handle);
  }

  auto add_portfolio(this Store& self, Portfolio portfolio)
   -> std::optional<PortfolioStoreHandle>
  {
    auto& portfolios = self.arena_.portfolios();
    auto& resolver = self.descriptor_.portfolio_store_data_resolver();
    return resolver.add(portfolios, std::move(portfolio));
  }

  auto get_portfolio(this const Store& self,
                     PortfolioStoreHandle handle) noexcept -> const Portfolio&
  {
    return self.descriptor_.portfolio_store_data_resolver().get(
     self.arena_.portfolios(), handle);
  }

  auto get_portfolio(this Store& self, PortfolioStoreHandle handle) noexcept
   -> Portfolio&
  {
    return self.descriptor_.portfolio_store_data_resolver().get(
     self.arena_.portfolios(), handle);
  }

  auto get_portfolio_if_present(this const Store& self,
                                PortfolioStoreHandle handle) noexcept
   -> const Portfolio*
  {
    return self.descriptor_.portfolio_store_data_resolver().get_if_present(
     self.arena_.portfolios(), handle);
  }

  auto get_portfolio_if_present(this Store& self,
                                PortfolioStoreHandle handle) noexcept
   -> Portfolio*
  {
    return self.descriptor_.portfolio_store_data_resolver().get_if_present(
     self.arena_.portfolios(), handle);
  }

  auto update_portfolio(this Store& self,
                        PortfolioStoreHandle handle,
                        Portfolio portfolio) -> bool
  {
    return self.descriptor_.portfolio_store_data_resolver().update(
     self.arena_.portfolios(), handle, std::move(portfolio));
  }

  auto remove_portfolio(this Store& self, PortfolioStoreHandle handle) -> bool
  {
    return self.descriptor_.portfolio_store_data_resolver().remove(
     self.arena_.portfolios(), handle);
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

  auto add_watchlist(this Store& self, Watchlist watchlist)
   -> std::optional<WatchlistStoreHandle>
  {
    return self.descriptor_.watchlist_store_data_resolver().add(
     self.arena_.watchlists(), std::move(watchlist));
  }

  auto get_watchlist(this const Store& self,
                     WatchlistStoreHandle handle) noexcept -> const Watchlist&
  {
    return self.descriptor_.watchlist_store_data_resolver().get(
     self.arena_.watchlists(), handle);
  }

  auto get_watchlist(this Store& self, WatchlistStoreHandle handle) noexcept
   -> Watchlist&
  {
    return self.descriptor_.watchlist_store_data_resolver().get(
     self.arena_.watchlists(), handle);
  }

  auto get_watchlist_if_present(this const Store& self,
                                WatchlistStoreHandle handle) noexcept
   -> const Watchlist*
  {
    return self.descriptor_.watchlist_store_data_resolver().get_if_present(
     self.arena_.watchlists(), handle);
  }

  auto get_watchlist_if_present(this Store& self,
                                WatchlistStoreHandle handle) noexcept
   -> Watchlist*
  {
    return self.descriptor_.watchlist_store_data_resolver().get_if_present(
     self.arena_.watchlists(), handle);
  }

  auto update_watchlist(this Store& self,
                        WatchlistStoreHandle handle,
                        Watchlist watchlist) -> bool
  {
    return self.descriptor_.watchlist_store_data_resolver().update(
     self.arena_.watchlists(), handle, std::move(watchlist));
  }

  auto remove_watchlist(this Store& self, WatchlistStoreHandle handle) -> bool
  {
    return self.descriptor_.watchlist_store_data_resolver().remove(
     self.arena_.watchlists(), handle);
  }

  auto add_model(this Store& self, Model model)
   -> std::optional<ModelStoreHandle>
  {
    auto& models = self.arena_.models();
    auto& model_resolver = self.descriptor_.model_store_data_resolver();

    return model_resolver.add(models, std::move(model));
  }

  auto get_model(this const Store& self, ModelStoreHandle handle) noexcept
   -> const Model&
  {
    const auto& models = self.arena_.models();
    const auto& model_resolver = self.descriptor_.model_store_data_resolver();
    return model_resolver.get(models, handle);
  }

  auto get_model(this Store& self, ModelStoreHandle handle) noexcept
   -> Model&
  {
    auto& models = self.arena_.models();
    auto& model_resolver = self.descriptor_.model_store_data_resolver();
    return model_resolver.get(models, handle);
  }

  auto get_model_if_present(this const Store& self,
                                ModelStoreHandle handle) noexcept
   -> const Model*
  {
    const auto& models = self.arena_.models();
    const auto& model_resolver = self.descriptor_.model_store_data_resolver();
    return model_resolver.get_if_present(models, handle);
  }

  auto get_model_if_present(this Store& self,
                                ModelStoreHandle handle) noexcept -> Model*
  {
    auto& models = self.arena_.models();
    auto& model_resolver = self.descriptor_.model_store_data_resolver();
    return model_resolver.get_if_present(models, handle);
  }

  auto update_model(this Store& self,
                        ModelStoreHandle handle,
                        Model model) -> bool
  {
    auto& models = self.arena_.models();
    auto& model_resolver = self.descriptor_.model_store_data_resolver();
    return model_resolver.update(models, handle, std::move(model));
  }

  auto remove_model(this Store& self, ModelStoreHandle handle) -> bool
  {
    auto& models = self.arena_.models();
    auto& model_resolver = self.descriptor_.model_store_data_resolver();
    return model_resolver.remove(models, handle);
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

  auto add_strategy(this Store& self, Strategy strategy)
   -> std::optional<StrategyStoreHandle>
  {
    return self.descriptor_.strategy_store_data_resolver().add(
     self.arena_.strategies(), std::move(strategy));
  }

  auto get_strategy(this const Store& self, StrategyStoreHandle handle) noexcept
   -> const Strategy&
  {
    return self.descriptor_.strategy_store_data_resolver().get(
     self.arena_.strategies(), handle);
  }

  auto get_strategy(this Store& self, StrategyStoreHandle handle) noexcept
   -> Strategy&
  {
    return self.descriptor_.strategy_store_data_resolver().get(
     self.arena_.strategies(), handle);
  }

  auto get_strategy_if_present(this const Store& self,
                               StrategyStoreHandle handle) noexcept -> const Strategy*
  {
    return self.descriptor_.strategy_store_data_resolver().get_if_present(
     self.arena_.strategies(), handle);
  }

  auto get_strategy_if_present(this Store& self,
                               StrategyStoreHandle handle) noexcept -> Strategy*
  {
    return self.descriptor_.strategy_store_data_resolver().get_if_present(
     self.arena_.strategies(), handle);
  }

  auto update_strategy(this Store& self,
                       StrategyStoreHandle handle,
                       Strategy strategy) -> bool
  {
    return self.descriptor_.strategy_store_data_resolver().update(
     self.arena_.strategies(), handle, std::move(strategy));
  }

  auto remove_strategy(this Store& self, StrategyStoreHandle handle) -> bool
  {
    return self.descriptor_.strategy_store_data_resolver().remove(
     self.arena_.strategies(), handle);
  }

  auto add_portfolio_results(this Store& self,
                             PortfolioStoreHandle handle,
                             PortfolioResults results) -> bool
  {
    return self.descriptor_.portfolio_results_store_data_resolver().add(
     self.arena_.portfolio_results(), handle, std::move(results));
  }

  auto get_portfolio_results(this const Store& self,
                             PortfolioStoreHandle handle) noexcept
   -> const PortfolioResults&
  {
    return self.descriptor_.portfolio_results_store_data_resolver().get(
     self.arena_.portfolio_results(), handle);
  }

  auto get_portfolio_results(this Store& self,
                             PortfolioStoreHandle handle) noexcept
   -> PortfolioResults&
  {
    return self.descriptor_.portfolio_results_store_data_resolver().get(
     self.arena_.portfolio_results(), handle);
  }

  auto get_portfolio_results_if_present(this const Store& self,
                                        PortfolioStoreHandle handle) noexcept
   -> const PortfolioResults*
  {
    return self.descriptor_.portfolio_results_store_data_resolver()
     .get_if_present(self.arena_.portfolio_results(), handle);
  }

  auto get_portfolio_results_if_present(this Store& self,
                                        PortfolioStoreHandle handle) noexcept
   -> PortfolioResults*
  {
    return self.descriptor_.portfolio_results_store_data_resolver()
     .get_if_present(self.arena_.portfolio_results(), handle);
  }

  auto get_or_create_portfolio_results(this Store& self,
                                       PortfolioStoreHandle handle)
   -> PortfolioResults&
  {
    auto* results = self.get_portfolio_results_if_present(handle);
    if(!results) {
      self.add_portfolio_results(handle, PortfolioResults{});
      results = self.get_portfolio_results_if_present(handle);
    }
    return *results;
  }

  auto update_portfolio_results(this Store& self,
                                PortfolioStoreHandle handle,
                                PortfolioResults results) -> bool
  {
    return self.descriptor_.portfolio_results_store_data_resolver().update(
     self.arena_.portfolio_results(), handle, std::move(results));
  }

  auto remove_portfolio_results(this Store& self, PortfolioStoreHandle handle)
   -> bool
  {
    return self.descriptor_.portfolio_results_store_data_resolver().remove(
     self.arena_.portfolio_results(), handle);
  }

private:
  StoreDescriptor descriptor_;
  StoreArena arena_;
};

} // namespace pludux::backtest
