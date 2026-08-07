module;

#include <utility>
#include <vector>

export module pludux.backtest:store_descriptor;

import pludux;

import :backtest;
import :portfolio;
import :portfolio_results;
import :asset;
import :watchlist;
import :strategy;
import :market;
import :broker;
import :profile;

import :store_handle;
import :store_data_resolver;

export namespace pludux::backtest {

class StoreDescriptor {
public:
  StoreDescriptor() = default;

  StoreDescriptor(
   StoreDataResolver<Backtest, BacktestStoreHandle>
    backtest_store_data_resolver,
   StoreDataResolver<Portfolio, PortfolioStoreHandle>
    portfolio_store_data_resolver,
   StoreDataResolver<Asset, AssetStoreHandle> asset_store_data_resolver,
   StoreDataResolver<Watchlist, WatchlistStoreHandle>
    watchlist_store_data_resolver,
   StoreDataResolver<Strategy, StrategyStoreHandle>
    strategy_store_data_resolver,
   StoreDataResolver<Market, MarketStoreHandle> market_store_data_resolver,
   StoreDataResolver<Broker, BrokerStoreHandle> broker_store_data_resolver,
   StoreDataResolver<Profile, ProfileStoreHandle> profile_store_data_resolver,
   StoreDataResolver<PortfolioResults, PortfolioStoreHandle>
    portfolio_results_store_data_resolver)
  : backtest_store_data_resolver_{std::move(backtest_store_data_resolver)}
  , portfolio_store_data_resolver_{std::move(portfolio_store_data_resolver)}
  , asset_store_data_resolver_{std::move(asset_store_data_resolver)}
  , watchlist_store_data_resolver_{std::move(watchlist_store_data_resolver)}
  , strategy_store_data_resolver_{std::move(strategy_store_data_resolver)}
  , market_store_data_resolver_{std::move(market_store_data_resolver)}
  , broker_store_data_resolver_{std::move(broker_store_data_resolver)}
  , profile_store_data_resolver_{std::move(profile_store_data_resolver)}
  , portfolio_results_store_data_resolver_{
     std::move(portfolio_results_store_data_resolver)}
  {
  }

  auto backtest_store_data_resolver(this const StoreDescriptor& self) noexcept
   -> const StoreDataResolver<Backtest, BacktestStoreHandle>&
  {
    return self.backtest_store_data_resolver_;
  }

  auto backtest_store_data_resolver(this StoreDescriptor& self) noexcept
   -> StoreDataResolver<Backtest, BacktestStoreHandle>&
  {
    return self.backtest_store_data_resolver_;
  }

  auto portfolio_store_data_resolver(this const StoreDescriptor& self) noexcept
   -> const StoreDataResolver<Portfolio, PortfolioStoreHandle>&
  {
    return self.portfolio_store_data_resolver_;
  }

  auto portfolio_store_data_resolver(this StoreDescriptor& self) noexcept
   -> StoreDataResolver<Portfolio, PortfolioStoreHandle>&
  {
    return self.portfolio_store_data_resolver_;
  }

  auto asset_store_data_resolver(this const StoreDescriptor& self) noexcept
   -> const StoreDataResolver<Asset, AssetStoreHandle>&
  {
    return self.asset_store_data_resolver_;
  }

  auto asset_store_data_resolver(this StoreDescriptor& self) noexcept
   -> StoreDataResolver<Asset, AssetStoreHandle>&
  {
    return self.asset_store_data_resolver_;
  }

  auto watchlist_store_data_resolver(this const StoreDescriptor& self) noexcept
   -> const StoreDataResolver<Watchlist, WatchlistStoreHandle>&
  {
    return self.watchlist_store_data_resolver_;
  }

  auto watchlist_store_data_resolver(this StoreDescriptor& self) noexcept
   -> StoreDataResolver<Watchlist, WatchlistStoreHandle>&
  {
    return self.watchlist_store_data_resolver_;
  }

  auto strategy_store_data_resolver(this const StoreDescriptor& self) noexcept
   -> const StoreDataResolver<Strategy, StrategyStoreHandle>&
  {
    return self.strategy_store_data_resolver_;
  }

  auto strategy_store_data_resolver(this StoreDescriptor& self) noexcept
   -> StoreDataResolver<Strategy, StrategyStoreHandle>&
  {
    return self.strategy_store_data_resolver_;
  }

  auto market_store_data_resolver(this const StoreDescriptor& self) noexcept
   -> const StoreDataResolver<Market, MarketStoreHandle>&
  {
    return self.market_store_data_resolver_;
  }

  auto market_store_data_resolver(this StoreDescriptor& self) noexcept
   -> StoreDataResolver<Market, MarketStoreHandle>&
  {
    return self.market_store_data_resolver_;
  }

  auto broker_store_data_resolver(this const StoreDescriptor& self) noexcept
   -> const StoreDataResolver<Broker, BrokerStoreHandle>&
  {
    return self.broker_store_data_resolver_;
  }

  auto broker_store_data_resolver(this StoreDescriptor& self) noexcept
   -> StoreDataResolver<Broker, BrokerStoreHandle>&
  {
    return self.broker_store_data_resolver_;
  }

  auto profile_store_data_resolver(this const StoreDescriptor& self) noexcept
   -> const StoreDataResolver<Profile, ProfileStoreHandle>&
  {
    return self.profile_store_data_resolver_;
  }

  auto profile_store_data_resolver(this StoreDescriptor& self) noexcept
   -> StoreDataResolver<Profile, ProfileStoreHandle>&
  {
    return self.profile_store_data_resolver_;
  }

  auto portfolio_results_store_data_resolver(
   this const StoreDescriptor& self) noexcept
   -> const StoreDataResolver<PortfolioResults, PortfolioStoreHandle>&
  {
    return self.portfolio_results_store_data_resolver_;
  }

  auto
  portfolio_results_store_data_resolver(this StoreDescriptor& self) noexcept
   -> StoreDataResolver<PortfolioResults, PortfolioStoreHandle>&
  {
    return self.portfolio_results_store_data_resolver_;
  }

private:
  StoreDataResolver<Backtest, BacktestStoreHandle>
   backtest_store_data_resolver_;
  StoreDataResolver<Portfolio, PortfolioStoreHandle>
   portfolio_store_data_resolver_;
  StoreDataResolver<Asset, AssetStoreHandle> asset_store_data_resolver_;
  StoreDataResolver<Watchlist, WatchlistStoreHandle>
   watchlist_store_data_resolver_;
  StoreDataResolver<Strategy, StrategyStoreHandle>
   strategy_store_data_resolver_;
  StoreDataResolver<Market, MarketStoreHandle> market_store_data_resolver_;
  StoreDataResolver<Broker, BrokerStoreHandle> broker_store_data_resolver_;
  StoreDataResolver<Profile, ProfileStoreHandle> profile_store_data_resolver_;

  StoreDataResolver<PortfolioResults, PortfolioStoreHandle>
   portfolio_results_store_data_resolver_;
};

} // namespace pludux::backtest
