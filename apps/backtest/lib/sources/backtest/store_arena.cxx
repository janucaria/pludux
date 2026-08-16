module;

#include <utility>
#include <vector>

export module pludux.backtest:store_arena;

import pludux;

import :system;
import :portfolio;
import :portfolio_results;
import :asset;
import :watchlist;
import :model;
import :market;
import :broker;
import :profile;
import :strategy;

export namespace pludux::backtest {

class StoreArena {
public:
  StoreArena() = default;

   StoreArena(std::vector<System> systems,
             std::vector<Portfolio> portfolios,
             std::vector<Asset> assets,
             std::vector<Watchlist> watchlists,
              std::vector<Model> models,
             std::vector<Market> markets,
             std::vector<Broker> brokers,
             std::vector<Profile> profiles,
             std::vector<Strategy> strategies,
             std::vector<PortfolioResults> portfolio_results)
   : systems_{std::move(systems)}
  , portfolios_{std::move(portfolios)}
  , assets_{std::move(assets)}
  , watchlists_{std::move(watchlists)}
  , models_{std::move(models)}
  , markets_{std::move(markets)}
  , brokers_{std::move(brokers)}
  , profiles_{std::move(profiles)}
   , strategies_{std::move(strategies)}
  , portfolio_results_{std::move(portfolio_results)}
  {
  }

  auto systems(this const StoreArena& self) noexcept
     -> const std::vector<System>&
  {
    return self.systems_;
  }

  auto systems(this StoreArena& self) noexcept -> std::vector<System>&
  {
    return self.systems_;
  }

  auto portfolios(this const StoreArena& self) noexcept
   -> const std::vector<Portfolio>&
  {
    return self.portfolios_;
  }

  auto portfolios(this StoreArena& self) noexcept -> std::vector<Portfolio>&
  {
    return self.portfolios_;
  }

  auto assets(this const StoreArena& self) noexcept -> const std::vector<Asset>&
  {
    return self.assets_;
  }

  auto assets(this StoreArena& self) noexcept -> std::vector<Asset>&
  {
    return self.assets_;
  }

  auto watchlists(this const StoreArena& self) noexcept
   -> const std::vector<Watchlist>&
  {
    return self.watchlists_;
  }

  auto watchlists(this StoreArena& self) noexcept -> std::vector<Watchlist>&
  {
    return self.watchlists_;
  }

  auto models(this const StoreArena& self) noexcept
    -> const std::vector<Model>&
  {
    return self.models_;
  }

   auto models(this StoreArena& self) noexcept -> std::vector<Model>&
  {
    return self.models_;
  }

  auto markets(this const StoreArena& self) noexcept
   -> const std::vector<Market>&
  {
    return self.markets_;
  }

  auto markets(this StoreArena& self) noexcept -> std::vector<Market>&
  {
    return self.markets_;
  }

  auto brokers(this const StoreArena& self) noexcept
   -> const std::vector<Broker>&
  {
    return self.brokers_;
  }

  auto brokers(this StoreArena& self) noexcept -> std::vector<Broker>&
  {
    return self.brokers_;
  }

  auto profiles(this const StoreArena& self) noexcept
   -> const std::vector<Profile>&
  {
    return self.profiles_;
  }

  auto profiles(this StoreArena& self) noexcept -> std::vector<Profile>&
  {
    return self.profiles_;
  }

  auto strategies(this const StoreArena& self) noexcept
   -> const std::vector<Strategy>&
  {
    return self.strategies_;
  }

  auto strategies(this StoreArena& self) noexcept -> std::vector<Strategy>&
  {
    return self.strategies_;
  }

  auto portfolio_results(this const StoreArena& self) noexcept
   -> const std::vector<PortfolioResults>&
  {
    return self.portfolio_results_;
  }

  auto portfolio_results(this StoreArena& self) noexcept
   -> std::vector<PortfolioResults>&
  {
    return self.portfolio_results_;
  }

private:
  std::vector<System> systems_;
  std::vector<Portfolio> portfolios_;
  std::vector<Asset> assets_;
  std::vector<Watchlist> watchlists_;
  std::vector<Model> models_;
  std::vector<Market> markets_;
  std::vector<Broker> brokers_;
  std::vector<Profile> profiles_;
  std::vector<Strategy> strategies_;
  std::vector<PortfolioResults> portfolio_results_;
};

} // namespace pludux::backtest
